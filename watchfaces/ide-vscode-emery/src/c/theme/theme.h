/**
 * @file theme.h
 * @brief Theme to background-resource and editor text-color mapping.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

/** @addtogroup watchface-vscode @{ */

/**
 * @brief Maps the Theme setting (0=Dark, 1=Light, 2=Terminal) to its baked background resource.
 *
 * @param theme The theme setting value.
 * @return The background resource ID.
 */
uint32_t bg_resource_for_theme(uint8_t theme);

/**
 * @brief The clock color for a theme (VS Code's keyword blue, Terminal's phosphor green).
 *
 * @param theme The theme setting value.
 * @return The time text color.
 */
GColor time_color_for_theme(uint8_t theme);

/**
 * @brief The day/date line color for a theme (VS Code's string orange/red, Terminal green).
 *
 * @param theme The theme setting value.
 * @return The date text color.
 */
GColor date_color_for_theme(uint8_t theme);

/**
 * @brief The stats color for a theme, used for the battery percentage and the terminal temp.
 *
 * @param theme The theme setting value.
 * @return The stats text color.
 */
GColor stats_color_for_theme(uint8_t theme);

/**
 * @brief The steps color for a theme (VS Code's light-blue variable, Terminal green).
 *
 * @param theme The theme setting value.
 * @return The steps text color.
 */
GColor steps_color_for_theme(uint8_t theme);

/**
 * @brief The heart-rate color for a theme (a red bpm tone, green on Terminal, white on Mono).
 *
 * @param theme The theme setting value.
 * @return The heart-rate text color.
 */
GColor hr_color_for_theme(uint8_t theme);

/** @} */
