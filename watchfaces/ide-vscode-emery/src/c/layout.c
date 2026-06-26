/**
 * @file layout.c
 * @brief ide-vscode-emery face: the zone table plus the WatchfaceDescriptor hooks that wire
 * the shell to this face's Share Tech Mono fonts, baked VS Code frame, and the
 * widget-drawn battery percentage.
 */
#include "layout.h"

#include "fonts.h"
#include "zone.h"
#include "theme/theme.h"
#include "widgets/widgets.h"

// --- Zone table ---
// the readouts the shell owns: the hero clock with the date centered beneath it,
// plus the terminal 2x2 grid (weather condition / temp on the left, heart rate /
// steps on the right). colours are seeded with the Dark+ palette and re-set per
// theme by apply_theme_colors
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]    = {.rect = SLOT_TIME, .font_id = FONT_TIME,  .align = GTextAlignmentCenter, .color = GColorVividCerulean,
                      // the @beats token is wider than HH:MM and overflows Teko 78; drop to 72 so it fits
                      .font_id_fallback = FONT_TIME_SM, .rect_fallback = SLOT_TIME},
    [ZONE_DATE]    = {.rect = SLOT_DATE,  .font_id = FONT_DATE, .align = GTextAlignmentCenter, .color = GColorRajah,
                      .font_id_fallback = FONT_VALUE, .rect_fallback = SLOT_DATE},
    [ZONE_COND]    = {.rect = SLOT_COND,  .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorMediumAquamarine,
                      .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_COND},
    [ZONE_WEATHER] = {.rect = SLOT_TEMP,  .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorMediumAquamarine},
    [ZONE_HR]      = {.rect = SLOT_HR,    .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorRed},
    [ZONE_STEPS]   = {.rect = SLOT_STEPS, .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorCeleste},
};

/**
 * @brief Returns the face's zone table.
 *
 * @return A pointer to the face's static zone table.
 */
static const Zone *face_zones(void)
{
    return s_zones;
}

/**
 * @brief Set the editor zone text colors to the current theme's palette.
 *
 * @param theme The theme setting value.
 */
static void apply_theme_colors(uint8_t theme)
{
    s_zones[ZONE_TIME].color    = time_color_for_theme(theme);
    s_zones[ZONE_DATE].color    = date_color_for_theme(theme);
    s_zones[ZONE_COND].color    = stats_color_for_theme(theme);
    s_zones[ZONE_WEATHER].color = stats_color_for_theme(theme);
    s_zones[ZONE_HR].color      = hr_color_for_theme(theme);
    s_zones[ZONE_STEPS].color   = steps_color_for_theme(theme);
}

// --- Fonts ---
/**
 * @brief The hero clock uses condensed Teko (tall + narrow); the terminal readouts
 * use Share Tech Mono at the sizes each slot needs.
 */
static void load_fonts(void)
{
    fonts_register(FONT_TIME,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_78)));
    fonts_register(FONT_TIME_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_72)));
    fonts_register(FONT_DATE,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_20)));
    fonts_register(FONT_VALUE,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_14)));
    fonts_register(FONT_DATE_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_12)));
}

// --- Baked VS Code frame ---
static BitmapLayer *s_frame_layer;
static GBitmap *s_frame_bitmap;
// 0xFF = nothing loaded yet (forces first load)
static uint8_t s_loaded_theme = 0xFF;

/**
 * @brief Load the baked frame for a theme.
 *
 * No-op when that theme is already loaded, so it's cheap to call on every
 * settings save.
 *
 * @param theme The theme setting value.
 */
static void load_frame(uint8_t theme)
{
    if (theme == s_loaded_theme && s_frame_bitmap)
    {
        return;
    }

    s_loaded_theme = theme;

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    s_frame_bitmap = gbitmap_create_with_resource(bg_resource_for_theme(theme));

    if (s_frame_layer)
    {
        bitmap_layer_set_bitmap(s_frame_layer, s_frame_bitmap);
    }
}

/**
 * @brief The frame bitmap sits at the bottom, the painted overlays just above it.
 *
 * @param parent The parent layer to add the scene to.
 * @param theme The theme setting value.
 */
static void create_scene(Layer *parent, uint8_t theme)
{
    apply_theme_colors(theme);

    s_frame_layer = bitmap_layer_create(layer_get_bounds(parent));
    layer_add_child(parent, bitmap_layer_get_layer(s_frame_layer));
    load_frame(theme);

    widgets_create(parent);
}

/**
 * @brief Swap the frame background and refresh the dynamic zone colors.
 *
 * @param theme The new theme setting value.
 */
static void update_theme(uint8_t theme)
{
    apply_theme_colors(theme);
    load_frame(theme);
}

/**
 * @brief Destroy the scene and unload fonts/bitmaps.
 */
static void destroy_scene(void)
{
    widgets_destroy();
    bitmap_layer_destroy(s_frame_layer);

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    fonts_unload_all();

    // force a reload if the window is recreated
    s_frame_layer = NULL;
    s_loaded_theme = 0xFF;
}

static const WatchfaceDescriptor s_face = {
    .zones = face_zones,
    .load_fonts = load_fonts,
    .create_scene = create_scene,
    .destroy_scene = destroy_scene,
    .update_theme = update_theme,
    .set_battery = widgets_set_battery,
    .set_weather_icon = widgets_set_weather_icon,
    .set_bluetooth = widgets_set_bluetooth,
    .refresh_overlays = widgets_mark_labels_dirty,
};

const WatchfaceDescriptor *vscode_emery_face(void)
{
    return &s_face;
}
