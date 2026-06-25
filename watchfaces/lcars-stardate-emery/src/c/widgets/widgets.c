/**
 * @file widgets.c
 * @brief Painted overlays: battery gauge, bar labels, weather/heart/feet glyphs.
 *
 * @ingroup watchface-lcars
 */
#include "widgets.h"

#include "layout.h"
#include "settings/settings.h"
#include "theme/theme.h"
#include "weather/icons.h"

static Layer *s_labels_layer;   // WEATHER / HEARTRATE / STEPS labels
static Layer *s_battery_layer;  // battery gauge in the top-left block
static Layer *s_glyphs_layer;   // weather / heart / feet icons over the readouts

static GFont s_font_label;  // Antonio 10 - bar labels
static GBitmap *s_heart_bmp, *s_feet_bmp, *s_wx_bmp, *s_thermo_bmp;
static GBitmap *s_bt_on_bmp, *s_bt_off_bmp;
static BluetoothStatus s_bt_status = BT_HIDDEN;
static uint32_t s_wx_resource;  // resource id currently in s_wx_bmp, to skip redundant reloads
static int s_batt_level;

// --- LCARS skin primitives ---
// stateless painters (ctx in, pixels out) for the chrome only LCARS faces draw:
// the holder-box label and the segmented battery gauge

/** @addtogroup watchface-lcars @{ */

/**
 * @brief Draw a left-aligned label inside a black LCARS holder box.
 *
 * @param ctx The graphics context.
 * @param area The area to draw in.
 * @param text The label text.
 * @param font The font to use.
 * @param text_color The text color.
 * @param pad Padding inside the box.
 */
static void lcars_label(GContext *ctx, GRect area, const char *text, GFont font, GColor text_color, int pad)
{
    GSize sz = graphics_text_layout_get_content_size(text, font, area, GTextOverflowModeFill, GTextAlignmentLeft);

    int box_w = sz.w + pad * 2;
    if (box_w > area.size.w)
    {
        box_w = area.size.w;
    }

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(area.origin.x, area.origin.y, box_w, area.size.h), 0, GCornerNone);

    graphics_context_set_text_color(ctx, text_color);
    graphics_draw_text(ctx, text, font,
        GRect(area.origin.x + pad, area.origin.y - 1, box_w - pad, area.size.h + 4),
        GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

/**
 * @brief Draw the 5-segment battery gauge, colour-coded by level.
 *
 * @param ctx The graphics context.
 * @param area The bounding box of the battery icon.
 * @param level Battery charge level percentage.
 * @param accent_color The outline and nub color.
 */
static void lcars_battery_gauge(GContext *ctx, GRect area, int level, GColor accent_color)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    GRect body = area;
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, 3, area.size.h / 2);

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, body, 2, GCornersAll);
    graphics_fill_rect(ctx, nub, 1, GCornersRight);

    GRect inner = GRect(body.origin.x + 2, body.origin.y + 2, body.size.w - 4, body.size.h - 4);

    const int segments = 5, gap = 2;
    int seg_w = (inner.size.w - (segments - 1) * gap) / segments;
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    int lit = (level * segments + 50) / 100;
    if (lit == 0 && level > 0)
    {
        lit = 1;
    }

    GColor on = (level <= 20) ? GColorRed : (level <= 40) ? GColorChromeYellow : accent_color;

    for (int i = 0; i < segments; i++)
    {
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y, seg_w, inner.size.h);

        // lit cells are filled solid, empty cells are hollow - a black interior with
        // an accent outline, so the cell stays visible without reading as charged
        graphics_context_set_fill_color(ctx, i < lit ? on : GColorBlack);
        graphics_fill_rect(ctx, cell, 1, GCornersAll);

        if (i >= lit)
        {
            graphics_context_set_stroke_color(ctx, accent_color);
            graphics_draw_round_rect(ctx, cell, 1);
        }
    }
}

/**
 * @brief Label definition mapping an area to its string text.
 */
typedef struct
{
    GRect area;
    const char *text;
} LcarsLabel;

/**
 * @brief Paint the four LCARS bar labels.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void labels_update_proc(Layer *layer, GContext *ctx)
{
    const LcarsLabel labels[] = {
        {LBL_STARDATE, "STARDATE"},
        {LBL_WEATHER, "SENSORS"},
        {LBL_HR, "VITALS"},
        {LBL_STEPS, "TRAVERSAL"},
    };

    for (unsigned i = 0; i < ARRAY_LENGTH(labels); i++)
    {
        lcars_label(ctx, labels[i].area, labels[i].text, s_font_label, LCARS_LABEL_COLOR, 3);
    }
}

/**
 * @brief Battery icon drawn on top of the top-left LCARS block.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void battery_update_proc(Layer *layer, GContext *ctx)
{
    GColor accent = panel_accent_for_theme(settings_u8(SETTING_THEME));
    lcars_battery_gauge(ctx, GRect(6, 4, 32, 12), s_batt_level, accent);
}

/**
 * @brief Recolor a palettized glyph to `color`, keeping each entry's alpha so the
 * anti-aliased edges stay smooth.
 *
 * Lets a white master icon be drawn in any flat color (here black, to match the
 * panels' coordinate text) without shipping a per-color asset. No-op for
 * non-palettized bitmaps.
 *
 * @param bmp The bitmap to recolor.
 * @param color The new color to apply.
 */
static void tint_palette_bitmap(GBitmap *bmp, GColor color)
{
    if (!bmp)
    {
        return;
    }

    GColor *palette = gbitmap_get_palette(bmp);
    if (!palette)
    {
        return;
    }

    int entries;
    switch (gbitmap_get_format(bmp))
    {
        case GBitmapFormat1BitPalette: entries = 2; break;
        case GBitmapFormat2BitPalette: entries = 4; break;
        case GBitmapFormat4BitPalette: entries = 16; break;
        default: return;
    }

    for (int i = 0; i < entries; i++)
    {
        // recolor only the visible entries and keep their alpha, so partly
        // covered edge pixels take the new hue at their original coverage rather
        // than staying white
        if (palette[i].a != 0)
        {
            palette[i].r = color.r;
            palette[i].g = color.g;
            palette[i].b = color.b;
        }
    }
}

/**
 * @brief Blit a transparent icon bitmap, skipping nulls.
 *
 * @param ctx The graphics context.
 * @param bmp The bitmap to draw.
 * @param r The destination rectangle.
 */
static void draw_icon(GContext *ctx, GBitmap *bmp, GRect r)
{
    if (!bmp)
    {
        return;
    }

    // honor PNG transparency
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    graphics_draw_bitmap_in_rect(ctx, bmp, r);
}

/**
 * @brief Paint the weather / thermometer / heart / feet glyphs, plus the bluetooth
 * status glyph (drawn only when the indicator is on and per connection state).
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void glyphs_update_proc(Layer *layer, GContext *ctx)
{
    draw_icon(ctx, s_wx_bmp, WX_ICON);
    draw_icon(ctx, s_thermo_bmp, THERMO_ICON);
    draw_icon(ctx, s_heart_bmp, HR_ICON);
    draw_icon(ctx, s_feet_bmp, FEET_ICON);

    GBitmap *bt = (s_bt_status == BT_CONNECTED) ? s_bt_on_bmp
                : (s_bt_status == BT_DISCONNECTED) ? s_bt_off_bmp
                : NULL;
    draw_icon(ctx, bt, BT_ICON);
}

void widgets_create(Layer *parent)
{
    GRect bounds = layer_get_bounds(parent);

    s_font_label = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10));

    s_heart_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_HEART);
    s_feet_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_FEET);
    s_thermo_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_THERMOMETER);
    s_wx_resource = RESOURCE_ID_ICON_WI_NA;
    s_wx_bmp = gbitmap_create_with_resource(s_wx_resource);
    s_bt_on_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH);
    s_bt_off_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH_SLASH);

    // the master glyphs are white, so recolor them black to match the panel text
    tint_palette_bitmap(s_bt_on_bmp, GColorBlack);
    tint_palette_bitmap(s_bt_off_bmp, GColorBlack);

    s_battery_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(parent, s_battery_layer);

    s_labels_layer = layer_create(bounds);
    layer_set_update_proc(s_labels_layer, labels_update_proc);
    layer_add_child(parent, s_labels_layer);

    s_glyphs_layer = layer_create(bounds);
    layer_set_update_proc(s_glyphs_layer, glyphs_update_proc);
    layer_add_child(parent, s_glyphs_layer);
}

void widgets_destroy(void)
{
    layer_destroy(s_labels_layer);
    layer_destroy(s_battery_layer);
    layer_destroy(s_glyphs_layer);
    s_labels_layer = NULL;
    s_battery_layer = NULL;
    s_glyphs_layer = NULL;

    if (s_heart_bmp)
    {
        gbitmap_destroy(s_heart_bmp);
        s_heart_bmp = NULL;
    }

    if (s_feet_bmp)
    {
        gbitmap_destroy(s_feet_bmp);
        s_feet_bmp = NULL;
    }

    if (s_thermo_bmp)
    {
        gbitmap_destroy(s_thermo_bmp);
        s_thermo_bmp = NULL;
    }

    if (s_wx_bmp)
    {
        gbitmap_destroy(s_wx_bmp);
        s_wx_bmp = NULL;
    }

    if (s_bt_on_bmp)
    {
        gbitmap_destroy(s_bt_on_bmp);
        s_bt_on_bmp = NULL;
    }

    if (s_bt_off_bmp)
    {
        gbitmap_destroy(s_bt_off_bmp);
        s_bt_off_bmp = NULL;
    }

    fonts_unload_custom_font(s_font_label);
}

void widgets_set_battery(int level)
{
    s_batt_level = level;
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}

void widgets_set_weather_icon(const char *condition)
{
    uint32_t resource = wx_resource_for(condition);

    // the ~30-min refresh and every temp-unit re-request resend the same
    // condition, so skip the destroy/create + repaint when the icon is unchanged
    if (resource == s_wx_resource && s_wx_bmp)
    {
        return;
    }

    if (s_wx_bmp)
    {
        gbitmap_destroy(s_wx_bmp);
        s_wx_bmp = NULL;
    }

    s_wx_resource = resource;
    s_wx_bmp = gbitmap_create_with_resource(resource);

    if (s_glyphs_layer)
    {
        layer_mark_dirty(s_glyphs_layer);
    }
}

void widgets_set_bluetooth(BluetoothStatus status)
{
    s_bt_status = status;
    if (s_glyphs_layer)
    {
        layer_mark_dirty(s_glyphs_layer);
    }
}

void widgets_mark_labels_dirty(void)
{
    if (s_labels_layer)
    {
        layer_mark_dirty(s_labels_layer);
    }

    // the battery's healthy fill is theme-colored, so repaint it on theme change
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}

/** @} */
