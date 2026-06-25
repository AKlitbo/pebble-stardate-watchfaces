/**
 * @file units.h
 * @brief unit conversion + distance formatting helpers
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Current swatch internet time in .beats.
 *
 * @return Swatch internet time in .beats (0..999).
 */
int units_swatch_beats(void);

/**
 * @brief Ms until the next .beats boundary.
 *
 * @return Milliseconds remaining until next boundary.
 */
uint32_t units_ms_until_next_beat(void);

/**
 * @brief Format meters as "N.N MI" or "N.N KM".
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void units_format_distance(char *buffer, size_t size, int meters, bool miles);

/** @} */
