/**
 * @file widgets.c
 * @brief Painted overlays: the static frame labels and the battery gauge, drawn over
 * the baked frame.
 *
 * @ingroup watchface-radar
 */
#include "widgets.h"

#include "layout.h"
#include "fonts.h"
#include "theme/theme.h"
#include "settings/settings.h"

static Layer *s_labels_layer;   // TEMP / PULSE / RANGE
static Layer *s_battery_layer;  // battery gauge in the top-left
static Layer *s_bt_layer;       // bluetooth status glyph in the date-bar cell
static int s_batt_level;

static GBitmap *s_bt_on_bmp, *s_bt_off_bmp;
static BluetoothStatus s_bt_status = BT_HIDDEN;

/** @addtogroup watchface-radar @{ */

/**
 * @brief Label definition mapping an area to its string text.
 */
typedef struct
{
    GRect area;
    const char *text;
} Label;

/**
 * @brief Paint the static labels.
 *
 * TEMP / PULSE / RANGE captions in the panel accent, matching the frame chrome.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void labels_update_proc(Layer *layer, GContext *ctx)
{
    GFont font = fonts_get(FONT_XS);
    GColor accent = panel_accent_for_theme(settings_u8(SETTING_THEME));

    const Label labels[] = {
        {LBL_TEMP,  "TEMP"},
        {LBL_PULSE, "PULSE"},
        {LBL_RANGE, "RANGE"},
    };

    graphics_context_set_text_color(ctx, accent);
    for (unsigned i = 0; i < ARRAY_LENGTH(labels); i++)
    {
        graphics_draw_text(ctx, labels[i].text, font, labels[i].area,
            GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}

/**
 * @brief Battery outlined in the theme accent, with five level-coloured segments.
 *
 * (Theme primary healthy, amber low, red critical) and a little terminal nub.
 *
 * @param layer The layer being updated.
 * @param ctx The graphics context.
 */
static void battery_update_proc(Layer *layer, GContext *ctx)
{
    int level = s_batt_level;
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    GRect body = BATT_RECT;
    GColor accent = panel_accent_for_theme(settings_u8(SETTING_THEME));

    int nub_h = body.size.h / 2;
    GRect nub = GRect(body.origin.x + body.size.w, body.origin.y + (body.size.h - nub_h) / 2, 2, nub_h);
    graphics_context_set_fill_color(ctx, accent);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    graphics_context_set_stroke_color(ctx, accent);
    graphics_draw_rect(ctx, body);

    // five segments, lit from the left by charge level
    const int segments = 5;
    const int gap = 1;
    GRect inner = GRect(body.origin.x + 2, body.origin.y + 1, body.size.w - 4, body.size.h - 2);
    int seg_w = (inner.size.w - (segments - 1) * gap) / segments;
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    int lit = (level * segments + 50) / 100;  // round to nearest segment
    if (lit == 0 && level > 0)
    {
        lit = 1;  // never read empty while there's still charge
    }

    GColor on = (level <= 20) ? GColorRed : (level <= 40) ? GColorChromeYellow : primary_color_for_theme(settings_u8(SETTING_THEME));
    graphics_context_set_fill_color(ctx, on);

    for (int i = 0; i < lit; i++)
    {
        // inset 1px top and bottom so a dark margin reads as a line inside the outline
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y + 1, seg_w, inner.size.h - 2);
        graphics_fill_rect(ctx, cell, 0, GCornerNone);
    }
}

/**
 * @brief Blit the bluetooth glyph for the current status.
 *
 * Nothing when hidden/connected has no bitmap. Honors PNG transparency so
 * the white glyph sits on the cell.
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

void widgets_create(Layer *parent)
{
    GRect bounds = layer_get_bounds(parent);

    s_bt_on_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH);
    s_bt_off_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLUETOOTH_SLASH);

    s_battery_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(parent, s_battery_layer);

    s_labels_layer = layer_create(bounds);
    layer_set_update_proc(s_labels_layer, labels_update_proc);
    layer_add_child(parent, s_labels_layer);

    s_bt_layer = layer_create(bounds);
    layer_set_update_proc(s_bt_layer, bt_update_proc);
    layer_add_child(parent, s_bt_layer);
}

void widgets_destroy(void)
{
    layer_destroy(s_labels_layer);
    layer_destroy(s_battery_layer);
    layer_destroy(s_bt_layer);
    s_labels_layer = NULL;
    s_battery_layer = NULL;
    s_bt_layer = NULL;

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

void widgets_set_weather_icon(const char *condition)
{
}

void widgets_set_bluetooth(BluetoothStatus status)
{
    s_bt_status = status;
    if (s_bt_layer)
    {
        layer_mark_dirty(s_bt_layer);
    }
}

void widgets_mark_labels_dirty(void)
{
    if (s_labels_layer)
    {
        layer_mark_dirty(s_labels_layer);
    }

    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}

/** @} */
