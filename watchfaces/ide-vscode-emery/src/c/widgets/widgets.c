/**
 * @file widgets.c
 * @brief Painted overlays drawn over the baked VS Code frame: the battery percentage
 * in the editor stats row, and the bluetooth connection glyph in the status bar.
 */
#include "widgets.h"

#include "layout.h"
#include "fonts.h"
#include "theme/theme.h"
#include "settings/settings.h"
#include "settings/setting_values.h"
#include "weather/icons.h"

static Layer *s_battery_layer;  // battery percentage in the editor stats row
static int s_batt_level = -1;

static Layer *s_bt_layer;       // bluetooth glyph in the status bar (bottom-left)
static GBitmap *s_bt_on_bmp, *s_bt_off_bmp;
static BluetoothStatus s_bt_status = BT_HIDDEN;

static Layer *s_weather_layer;  // condition glyph in the terminal's top-left stat
static GBitmap *s_wx_bmp;
static uint32_t s_wx_resource;

/**
 * @brief Recolor a palettized glyph to `color`, keeping each entry's alpha so the
 * anti-aliased edges stay smooth. Lets one white master icon take any theme tint.
 *
 * @param bmp The bitmap to recolor.
 * @param color The flat color to apply.
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
        if (palette[i].a != 0)
        {
            palette[i].r = color.r;
            palette[i].g = color.g;
            palette[i].b = color.b;
        }
    }
}

/**
 * @brief Draw a battery icon outline with a fill that tracks the charge level.
 *
 * The body is an outlined rounded box with a nub on the right; the inner fill width
 * is proportional to `level` and flips to red when the charge is critical.
 *
 * @param ctx The graphics context.
 * @param area The battery body rect (the nub is drawn just past its right edge).
 * @param level Battery charge level percentage (0..100).
 * @param color The outline + fill color when not critical.
 */
static void draw_battery_icon(GContext *ctx, GRect area, int level, GColor color)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    // outlined body
    graphics_context_set_stroke_color(ctx, color);
    graphics_draw_rect(ctx, area);

    // nub on the right edge
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, 2, area.size.h / 2);
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    // proportional fill inside the body (1px inset), red when critical
    int inner_w = area.size.w - 2;
    int fill_w = (level * inner_w + 50) / 100;
    if (fill_w > 0)
    {
        GColor fill = (level <= 20) ? GColorRed : color;
        graphics_context_set_fill_color(ctx, fill);
        graphics_fill_rect(ctx, GRect(area.origin.x + 1, area.origin.y + 1, fill_w, area.size.h - 2), 0, GCornerNone);
    }
}

/**
 * @brief Battery icon + percentage, per the Battery Display setting. The percentage
 * flips to red when critical; the icon fills to the charge level.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void battery_update_proc(Layer *layer, GContext *ctx)
{
    int level = s_batt_level;
    if (level < 0)
    {
        return;  // no reading yet
    }

    if (level > 100)
    {
        level = 100;
    }

    uint8_t display = settings_u8(SETTING_BATTERY_DISPLAY);
    GColor color = (level <= 20) ? GColorRed : stats_color_for_theme(settings_u8(SETTING_THEME));

    if (display != BATTERY_DISPLAY_PERCENT)
    {
        draw_battery_icon(ctx, BATT_ICON, level, stats_color_for_theme(settings_u8(SETTING_THEME)));
    }

    if (display != BATTERY_DISPLAY_ICON)
    {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d%%", level);

        // with no icon, the percentage slides right into the icon's vacated space
        GRect rect = (display == BATTERY_DISPLAY_PERCENT) ? BATT_RECT_WIDE : BATT_RECT;

        graphics_context_set_text_color(ctx, color);
        graphics_draw_text(ctx, buffer, fonts_get(FONT_VALUE), rect,
            GTextOverflowModeFill, GTextAlignmentRight, NULL);
    }
}

/**
 * @brief Blit the bluetooth glyph for the current status.
 *
 * Draws nothing when hidden. Honors PNG transparency (GCompOpSet) so the glyph
 * sits over the baked status bar.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void bt_update_proc(Layer *layer, GContext *ctx)
{
    GBitmap *bmp = (s_bt_status == BT_CONNECTED) ? s_bt_on_bmp
                 : (s_bt_status == BT_DISCONNECTED) ? s_bt_off_bmp
                 : NULL;

    if (!bmp)
    {
        return;
    }

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, BT_ICON);
}

/**
 * @brief Weather condition glyph in the terminal's top-left stat. The white master
 * is tinted to the theme's stats color each paint, so a theme swap recolors it too.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void weather_update_proc(Layer *layer, GContext *ctx)
{
    if (!s_wx_bmp)
    {
        return;
    }

    tint_palette_bitmap(s_wx_bmp, stats_color_for_theme(settings_u8(SETTING_THEME)));

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_wx_bmp, WX_ICON);
}

void widgets_create(Layer *parent)
{
    GRect bounds = layer_get_bounds(parent);

    s_bt_on_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH);
    s_bt_off_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH_SLASH);

    s_wx_resource = RESOURCE_ID_ICON_WI_NA;
    s_wx_bmp = gbitmap_create_with_resource(s_wx_resource);

    s_battery_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(parent, s_battery_layer);

    s_weather_layer = layer_create(bounds);
    layer_set_update_proc(s_weather_layer, weather_update_proc);
    layer_add_child(parent, s_weather_layer);

    s_bt_layer = layer_create(bounds);
    layer_set_update_proc(s_bt_layer, bt_update_proc);
    layer_add_child(parent, s_bt_layer);
}

void widgets_destroy(void)
{
    layer_destroy(s_battery_layer);
    layer_destroy(s_weather_layer);
    layer_destroy(s_bt_layer);
    s_battery_layer = NULL;
    s_weather_layer = NULL;
    s_bt_layer = NULL;

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
}

void widgets_set_battery(int level)
{
    s_batt_level = level;
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}

void widgets_set_bluetooth(BluetoothStatus status)
{
    s_bt_status = status;
    if (s_bt_layer)
    {
        layer_mark_dirty(s_bt_layer);
    }
}

void widgets_set_weather_icon(const char *condition)
{
    uint32_t resource = wx_resource_for(condition);

    // the periodic refresh resends the same condition, so skip the reload + repaint
    // when the glyph is unchanged
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

    if (s_weather_layer)
    {
        layer_mark_dirty(s_weather_layer);
    }
}

void widgets_mark_labels_dirty(void)
{
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }

    if (s_weather_layer)
    {
        layer_mark_dirty(s_weather_layer);
    }

    if (s_bt_layer)
    {
        layer_mark_dirty(s_bt_layer);
    }
}
