/**
 * @file widgets.h
 * @brief Overlay widgets: the layers the shell mounts over the baked VS Code frame.
 *
 * This face's painted overlays are the battery percentage (next to the baked
 * battery glyph in the editor stats row), the weather condition glyph (top-left of
 * the terminal stats, swapped per condition and tinted to the theme), and the
 * bluetooth connection glyph (bottom-left of the status bar). Create them after the
 * background layer and before the text layers to preserve z-order.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "shell/shell.h"

/** @addtogroup watchface-vscode @{ */

/**
 * @brief Create the overlay layers.
 *
 * @param parent The parent layer.
 */
void widgets_create(Layer *parent);

/**
 * @brief Destroy the overlay layers and icon bitmaps.
 */
void widgets_destroy(void);

/**
 * @brief Set the battery level (0..100) and redraw the percentage.
 *
 * @param level Battery charge level percentage.
 */
void widgets_set_battery(int level);

/**
 * @brief Swap/hide the bluetooth connection glyph.
 *
 * @param status The new bluetooth connection status.
 */
void widgets_set_bluetooth(BluetoothStatus status);

/**
 * @brief Swap the weather condition glyph (top-left terminal stat) for a condition.
 *
 * @param condition The condition abbreviation string.
 */
void widgets_set_weather_icon(const char *condition);

/**
 * @brief Force an overlay repaint (e.g. after a theme change recolors the readout).
 */
void widgets_mark_labels_dirty(void);

/** @} */
