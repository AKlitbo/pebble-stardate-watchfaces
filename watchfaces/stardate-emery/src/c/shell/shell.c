// the stardate-emery LCARS shell: builds the window and renders every slot
#include "shell/shell.h"

#include "layout.h"
#include "settings/settings.h"
#include "theme/theme.h"
#include "draw/draw.h"
#include "units/units.h"
#include "weather/icons.h"
#include "widgets/widgets.h"

static Window *s_main_window;
static BitmapLayer *s_lcars_layer;
static GBitmap *s_lcars_bitmap;
static TextLayer *s_time_layer;      // .time slot
static TextLayer *s_meridiem_layer;  // AM/PM, above the time (12h modes only)
static TextLayer *s_date_layer;      // .banner slot (top strip)
static TextLayer *s_hr_layer;        // HEARTRATE value
static TextLayer *s_steps_layer;     // STEPS value (today's step count)
static TextLayer *s_weather_layer;   // WEATHER temperature
static TextLayer *s_cond_layer;      // weather condition abbreviation (RAIN/CLDY)
static TextLayer *s_lat_layer;       // latitude readout on the upper-left maroon block
static TextLayer *s_lon_layer;       // longitude readout on the lower-left maroon block

static GFont s_font_time;     // Antonio 62 - time
static GFont s_font_date;     // Antonio 36 - date
static GFont s_font_date_sm;  // Antonio 32 - date fallback for wide formats
static GFont s_font_sm;       // Antonio 20 - weather temp
static GFont s_font_value;    // Antonio 16 - heartrate / steps values
static GFont s_font_coord;    // Antonio 12 - lat / lon (narrow enough for the left bars)
static GFont s_font_xs;       // Antonio 10 - AM/PM superscript (12 is digits-only, no letters)

static uint8_t s_loaded_theme = 0xFF;  // 0xFF = nothing loaded yet (forces first load)

static char s_hr_buffer[8];
static char s_steps_buffer[12];

// last health readings, cached so the TRAVERSAL slot can be re-rendered when the
// TraversalMode setting changes without waiting for the next health/tick update
static int s_last_steps;
static int s_last_distance_m;
static char s_weather_temp[12];
static char s_weather_cond[12];
static char s_lat_buffer[12];
static char s_lon_buffer[12];

// re-render TRAVERSAL slot from cached readings
static void render_traversal(void);  

// (re)load the baked LCARS frame for the active theme. No-op when that theme is
// already loaded, so it's cheap to call on every settings save
static void load_background(void)
{
    if (g_settings.Theme == s_loaded_theme && s_lcars_bitmap)
    {
        return;
    }

    s_loaded_theme = g_settings.Theme;

    if (s_lcars_bitmap)
    {
        gbitmap_destroy(s_lcars_bitmap);
        s_lcars_bitmap = NULL;
    }

    s_lcars_bitmap = gbitmap_create_with_resource(bg_resource_for_theme(g_settings.Theme));
    
    if (s_lcars_layer)
    {
        bitmap_layer_set_bitmap(s_lcars_layer, s_lcars_bitmap);
    }
}

// repaint every readout in the LCARS palette and reflect any changed settings
void shell_update_display(void)
{
    if (!s_main_window)
    {
        return;
    }

    // swap the frame if the Theme setting changed
    load_background();  

    // fixed LCARS palette: black field, white readouts (theme colour settings removed)
    window_set_background_color(s_main_window, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text_color(s_meridiem_layer, GColorWhite);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_text_color(s_hr_layer, GColorWhite);
    text_layer_set_text_color(s_steps_layer, GColorWhite);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_color(s_cond_layer, GColorWhite);
    // coordinates sit on the colored left-rail bars, so they're black like the
    // LCARS numerals - readable on every theme's blocks (luminance 79–180)
    text_layer_set_text_color(s_lat_layer, COORD_TEXT_COLOR);
    text_layer_set_text_color(s_lon_layer, COORD_TEXT_COLOR);
    render_traversal();  // reflect a changed TraversalMode immediately
    widgets_mark_labels_dirty();
}

// fit the date to the banner box: Antonio 36 if it fits, else fall back to
// Antonio 32 (with the recentred frame). measured by pixel width, not char
// count, because Antonio is proportional - "06/12/2026" clips where the
// narrower "2026.0613" fits at the same length
static void fit_date_layer(const char *text)
{
    // SLOT_BANNER width (142) minus a 2px safety margin
    const int avail = 140;

    GSize sz = graphics_text_layout_get_content_size(text, s_font_date, GRect(0, 0, 1000, 100),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

    if (sz.w <= avail)
    {
        text_layer_set_font(s_date_layer, s_font_date);  // Antonio 36
        layer_set_frame(text_layer_get_layer(s_date_layer), SLOT_BANNER);
    }
    else
    {
        text_layer_set_font(s_date_layer, s_font_date_sm);  // Antonio 32
        layer_set_frame(text_layer_get_layer(s_date_layer), SLOT_BANNER_SM);
    }
    text_layer_set_text(s_date_layer, text);
}

// render the clock, AM/PM, and date for the current settings
void shell_update_time(struct tm *tick_time)
{
    static char s_time_buffer[8];

    // Swatch .beats - replaces the clock
    if (g_settings.TimeFormat == 3)
    {
        snprintf(s_time_buffer, sizeof(s_time_buffer), "@%03d", units_swatch_beats());
    }
    else
    {
        // 24 -> 12 -> System
        const char *fmt = (g_settings.TimeFormat == 2) ? "%H:%M" 
            : (g_settings.TimeFormat == 1)
                ? "%I:%M"                                       
                : (clock_is_24h_style() ? "%H:%M" : "%I:%M");

        strftime(s_time_buffer, sizeof(s_time_buffer), fmt, tick_time);
    }

    text_layer_set_text(s_time_layer, s_time_buffer);

    // AM/PM only makes sense on a 12-hour clock (explicit 12-hour, or System when
    // the watch isn't in 24h mode). Cleared otherwise so it never shows on 24h/.beats
    if (s_meridiem_layer)
    {
        bool h12 =
            (g_settings.TimeFormat == 1) || (g_settings.TimeFormat == 0 && !clock_is_24h_style());
        text_layer_set_text(s_meridiem_layer, h12 ? (tick_time->tm_hour < 12 ? "AM" : "PM") : "");
    }

    static char s_date_buffer[24];
    char date_now[24];
    strftime(date_now, sizeof(date_now), g_settings.DateFormat, tick_time);
    draw_string_to_upper(date_now);

    // the date only changes at midnight (or when the format changes), but this runs
    // every tick/beat - skip the pixel-width fit + relayout unless the text differs
    if (strcmp(date_now, s_date_buffer) != 0)
    {
        strncpy(s_date_buffer, date_now, sizeof(s_date_buffer) - 1);
        s_date_buffer[sizeof(s_date_buffer) - 1] = '\0';
        fit_date_layer(s_date_buffer);
    }
}

// render the TRAVERSAL slot from the cached health readings per TraversalMode:
// steps, distance in miles, or distance in km (distance is one decimal place)
static void render_traversal(void)
{
    if (!s_steps_layer)
    {
        return;
    }
    if (g_settings.TraversalMode == 1 || g_settings.TraversalMode == 2)
    {
        units_format_distance(s_steps_buffer, sizeof(s_steps_buffer), s_last_distance_m,
                              g_settings.TraversalMode == 1);
    }
    else
    {
        snprintf(s_steps_buffer, sizeof(s_steps_buffer), "%d", s_last_steps);
    }
    text_layer_set_text(s_steps_layer, s_steps_buffer);
}

// update the heart-rate and traversal readouts from a health event
void shell_update_info(int hr, int steps, int distance_m)
{
    if (hr > 0)
    {
        snprintf(s_hr_buffer, sizeof(s_hr_buffer), "%d", hr);
    }
    else
    {
        snprintf(s_hr_buffer, sizeof(s_hr_buffer), "--");
    }
    if (s_hr_layer)
    {
        text_layer_set_text(s_hr_layer, s_hr_buffer);
    }

    s_last_steps = steps;
    s_last_distance_m = distance_m;
    render_traversal();
}

// redraw the battery gauge
void shell_set_battery(int level)
{
    widgets_set_battery(level);
}

// set the temp + condition text and swap the weather glyph
void shell_set_weather(const char *temp, const char *condition)
{
    strncpy(s_weather_temp, temp ? temp : "--", sizeof(s_weather_temp) - 1);
    s_weather_temp[sizeof(s_weather_temp) - 1] = '\0';

    if (s_weather_layer)
    {
        text_layer_set_text(s_weather_layer, s_weather_temp);
    }

    strncpy(s_weather_cond, condition ? condition : "--", sizeof(s_weather_cond) - 1);
    s_weather_cond[sizeof(s_weather_cond) - 1] = '\0';

    // drop the night marker so "CLEAR_NIGHT" fits as "CLEAR" (the icon below uses the full string)
    char *night = strchr(s_weather_cond, '_');
    if (night)
    {
        *night = '\0';
    }

    if (s_cond_layer)
    {
        text_layer_set_text(s_cond_layer, s_weather_cond);
    }

    widgets_set_weather_icon(condition);  // full string (incl. _NIGHT) drives the glyph
}

// set the lat/lon readouts, with a placeholder when GPS is off
void shell_set_coords(const char *lat, const char *lon)
{
    // JS pre-formats these in LCARS dash style ("33-44", "-112-07") - empty when
    // GPS is off or unavailable, in which case we show a placeholder
    strncpy(s_lat_buffer, (lat && lat[0]) ? lat : "--", sizeof(s_lat_buffer) - 1);
    s_lat_buffer[sizeof(s_lat_buffer) - 1] = '\0';

    if (s_lat_layer)
    {
        text_layer_set_text(s_lat_layer, s_lat_buffer);
    }

    strncpy(s_lon_buffer, (lon && lon[0]) ? lon : "--", sizeof(s_lon_buffer) - 1);
    s_lon_buffer[sizeof(s_lon_buffer) - 1] = '\0';

    if (s_lon_layer)
    {
        text_layer_set_text(s_lon_layer, s_lon_buffer);
    }
}

// build all layers, fonts, and overlays for the watchface window
static void main_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // LCARS frame - a background bitmap baked ahead of time from the per-theme
    // frame layout. The active theme (g_settings.Theme) selects which to load
    s_lcars_layer = bitmap_layer_create(bounds);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_lcars_layer));
    load_background();

    s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_62));
    s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_36));
    s_font_date_sm = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_32));
    s_font_sm = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_20));
    s_font_value = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_16));
    s_font_coord = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_12));
    s_font_xs = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10));

    // painted overlays over the background: battery, bar labels, icons
    widgets_create(window_layer);

    s_time_layer = draw_make_layer(window_layer, SLOT_TIME, s_font_time, GTextAlignmentCenter);
    s_meridiem_layer = draw_make_layer(window_layer, SLOT_MERIDIEM, s_font_xs, GTextAlignmentRight);
    s_date_layer = draw_make_layer(window_layer, SLOT_BANNER, s_font_date, GTextAlignmentCenter);
    s_weather_layer = draw_make_layer(window_layer, SLOT_WEATHER, s_font_sm, GTextAlignmentLeft);
    s_cond_layer = draw_make_layer(window_layer, SLOT_COND, s_font_value, GTextAlignmentLeft);
    s_hr_layer = draw_make_layer(window_layer, SLOT_HR, s_font_value, GTextAlignmentLeft);
    s_steps_layer = draw_make_layer(window_layer, SLOT_STEPS, s_font_value, GTextAlignmentLeft);
    s_lat_layer = draw_make_layer(window_layer, SLOT_LAT, s_font_coord, GTextAlignmentRight);
    s_lon_layer = draw_make_layer(window_layer, SLOT_LON, s_font_coord, GTextAlignmentRight);

    text_layer_set_text(s_weather_layer, "--");
    text_layer_set_text(s_cond_layer, "--");
    text_layer_set_text(s_lat_layer, "--");
    text_layer_set_text(s_lon_layer, "--");
    shell_update_display();
}

// destroy all layers, fonts, and overlays when the window unloads
static void main_window_unload(Window *window)
{
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_meridiem_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_hr_layer);
    text_layer_destroy(s_steps_layer);
    text_layer_destroy(s_weather_layer);
    text_layer_destroy(s_cond_layer);
    text_layer_destroy(s_lat_layer);
    text_layer_destroy(s_lon_layer);

    fonts_unload_custom_font(s_font_time);
    fonts_unload_custom_font(s_font_date);
    fonts_unload_custom_font(s_font_date_sm);
    fonts_unload_custom_font(s_font_sm);
    fonts_unload_custom_font(s_font_value);
    fonts_unload_custom_font(s_font_coord);
    fonts_unload_custom_font(s_font_xs);

    widgets_destroy();
    bitmap_layer_destroy(s_lcars_layer);
    
    if (s_lcars_bitmap)
    {
        gbitmap_destroy(s_lcars_bitmap);
    }

    // force a reload if the window is recreated
    s_lcars_bitmap = NULL;
    s_loaded_theme = 0xFF;
}

// create the main window and push it onto the stack
void shell_init(void)
{
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, 
        (WindowHandlers){.load = main_window_load, .unload = main_window_unload});
    window_stack_push(s_main_window, true);
}

// destroy the main window
void shell_deinit(void)
{
    window_destroy(s_main_window);
}
