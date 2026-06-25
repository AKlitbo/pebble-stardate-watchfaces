/**
 * @file fonts.h
 * @brief font registry: a face loads its custom fonts once under a category id, the rest
 * of the ui looks them up by id so layout tables can stay static (an id, not a
 * runtime GFont handle). single-platform binaries register their own handles, so
 * there is no per-platform font-set resolution here
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Identifier for each loaded font category.
 */
typedef enum
{
    FONT_TIME,    // Clock readout
    FONT_DATE,    // Date banner
    FONT_DATE_SM, // Date banner fallback for wide formats
    FONT_DATE_XS, // Date banner fallback for the widest formats
    FONT_SM,      // Weather temp
    FONT_VALUE,   // Heartrate / steps values
    FONT_COORD,   // Lat / lon
    FONT_XS,      // AM/PM superscript
    FONT_COUNT
} FontId;

/**
 * @brief Store a loaded font handle under its category id.
 *
 * @param id The category id.
 * @param handle The loaded font handle.
 */
void fonts_register(FontId id, GFont handle);

/**
 * @brief Resolve a category id to its handle; falls back to the system font on a miss.
 *
 * @param id The category id.
 * @return The font handle.
 */
GFont fonts_get(FontId id);

/**
 * @brief Unload every registered font and clear the registry.
 */
void fonts_unload_all(void);

/** @} */
