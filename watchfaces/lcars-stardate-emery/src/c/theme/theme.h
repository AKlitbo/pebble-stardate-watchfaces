/**
 * @file theme.h
 * @brief Theme to background-resource and accent-color mapping.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/** @addtogroup watchface-lcars @{ */

/**
 * @brief Maps the Theme setting (0=Classic, 1=Lower Decks, 2=PADD, 3=Nemesis Blue) to
 * its baked LCARS background resource.
 *
 * @param theme The theme setting value.
 * @return The background resource ID.
 */
uint32_t bg_resource_for_theme(uint8_t theme);

/**
 * @brief The theme's top-left cap color, so painted overlays (e.g. the battery gauge)
 * can match the panel they sit on instead of staying a fixed classic violet.
 *
 * @param theme The theme setting value.
 * @return The accent color.
 */
GColor panel_accent_for_theme(uint8_t theme);

/** @} */
