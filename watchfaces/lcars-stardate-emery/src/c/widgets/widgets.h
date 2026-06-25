/**
 * @file widgets.h
 * @brief Overlay widgets API: layers the shell mounts over the baked LCARS frame.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

#include "shell/shell.h"

/** @addtogroup watchface-lcars @{ */

/**
 * @brief Painted overlays drawn on top of the baked LCARS frame.
 *
 * Includes the battery gauge, the amber bar labels, and the weather / heart /
 * feet / bluetooth glyphs. This module owns the overlay layers plus their label
 * font and icon bitmaps - create it after the background layer and before the
 * text layers to preserve z-order.
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
 * @brief Swaps the weather glyph.
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
 * @brief Force a label repaint.
 */
void widgets_mark_labels_dirty(void);

/** @} */
