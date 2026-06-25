/**
 * @file distance.h
 * @brief pure distance conversions + formatting (no SDK)
 *
 * @ingroup lib
 */
#pragma once
#include <stddef.h>
#include <stdbool.h>

/** @addtogroup lib @{ */

/**
 * @brief Meters to miles.
 *
 * @param meters The distance in meters.
 * @return Distance in miles.
 */
float distance_m_to_miles(int meters);

/**
 * @brief Meters to kilometers.
 *
 * @param meters The distance in meters.
 * @return Distance in kilometers.
 */
float distance_m_to_km(int meters);

/**
 * @brief Format meters as "N.N MI" or "N.N KM" (rounded to tenth).
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void distance_format(char *buffer, size_t size, int meters, bool miles);

/** @} */
