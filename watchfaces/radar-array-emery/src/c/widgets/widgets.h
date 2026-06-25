/**
 * @file widgets.h
 * @brief Overlay widgets: the layers the shell mounts over the baked radar-array frame.
 *
 * This module owns the static labels (TEMP / PULSE / RANGE) and the battery
 * gauge - create it after the background layer and before the text layers to
 * preserve z-order. labels are drawn here (not baked) so a future locale can
 * swap the strings.
 *
 * @ingroup watchface-radar
 */
#pragma once
#include <pebble.h>

#include "shell/shell.h"

/** @addtogroup watchface-radar @{ */

/**
 * @brief Create the overlay layers.
 *
 * @param parent The parent layer.
 */
void widgets_create(Layer *parent);

/**
 * @brief Destroy the overlay layers, font, and icon bitmaps.
 */
void widgets_destroy(void);

/**
 * @brief Set the battery level (0..100) and redraw the gauge.
 *
 * @param level Battery charge level percentage.
 */
void widgets_set_battery(int level);

/**
 * @brief No glyph on this face (no-op).
 *
 * @param condition The weather condition abbreviation.
 */
void widgets_set_weather_icon(const char *condition);

/**
 * @brief Swap/hide the connection glyph.
 *
 * @param status The new bluetooth connection status.
 */
void widgets_set_bluetooth(BluetoothStatus status);

/**
 * @brief Force a label + gauge repaint.
 */
void widgets_mark_labels_dirty(void);

/** @} */
