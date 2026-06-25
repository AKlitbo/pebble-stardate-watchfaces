// the shell: a skin-agnostic orchestrator that places a face's zones and renders
// time / date / weather / health / coords into them. The face supplies a
// WatchfaceDescriptor - its zone table plus hooks for the things only it knows
/**
 * @file shell.h
 * @brief ui shell: handles the window lifecycle, layer placement, and ticking. faces
 * define how to paint the layout and decode the settings; the shell drives them
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>
#include "zone.h"
#include "appmessage/appmessage.h"

/**
 * @defgroup lib Library Components
 * @brief Core reusable components.
 * @{
 */

/**
 * @brief What the bluetooth glyph should show. BT_HIDDEN covers the case where the user
 * turned the indicator off, so the face draws nothing.
 */
typedef enum
{
    BT_HIDDEN,
    BT_CONNECTED,
    BT_DISCONNECTED
} BluetoothStatus;

/**
 * @brief The active face exposes its paint routines and zone table to the shell.
 */
typedef struct
{
    const Zone *(*zones)(void);                          /**< the table[ZONE_COUNT] */
    void (*load_fonts)(void);                            /**< register each FontId */
    void (*create_scene)(Layer *parent, uint8_t theme);  /**< frame + overlays, bottom z-order */
    void (*destroy_scene)(void);                         /**< tear down frame + overlays + fonts */
    void (*update_theme)(uint8_t theme);                 /**< swap the frame on a theme change */
    void (*set_battery)(int level);                      /**< redraw the battery gauge */
    void (*set_weather_icon)(const char *condition);     /**< swap the weather glyph */
    void (*set_bluetooth)(BluetoothStatus status);       /**< swap/hide the connection glyph */
    void (*refresh_overlays)(void);                      /**< repaint painted overlays */
} WatchfaceDescriptor;

/**
 * @brief Create main window for face and push onto stack.
 *
 * @param face The active watchface descriptor.
 */
void shell_init(const WatchfaceDescriptor *face);

/**
 * @brief Destroy main window.
 */
void shell_deinit(void);

/**
 * @brief Reflect changed settings (theme, steps mode, painted overlays).
 */
void shell_update_display(void);

/**
 * @brief Render clock, AM/PM, and date.
 *
 * @param tick_time The current time.
 */
void shell_update_time(struct tm *tick_time);

/**
 * @brief Update heart-rate and steps readouts.
 *
 * @param hr Heart rate in bpm.
 * @param steps Today's step count.
 * @param distance_m Today's distance walked in meters.
 */
void shell_update_info(int hr, int steps, int distance_m);

/**
 * @brief Redraw battery gauge.
 *
 * @param level Battery charge level percentage.
 */
void shell_set_battery(int level);

/**
 * @brief Set temp + condition text and swap weather glyph.
 *
 * @param temp The formatted temperature string.
 * @param condition The weather condition abbreviation.
 */
void shell_set_weather(const char *temp, const char *condition);

/**
 * @brief Set lat/lon readouts.
 *
 * @param lat Latitude string.
 * @param lon Longitude string.
 */
void shell_set_coords(const char *lat, const char *lon);

/** @} */
