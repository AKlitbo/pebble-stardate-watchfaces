/**
 * @file health.h
 * @brief health-service accessors (no-op stubs when PBL_HEALTH is absent)
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Subscribe to health events.
 *
 * @param handler The callback function.
 */
void health_init(HealthEventHandler handler);

/**
 * @brief Current heart rate in bpm (or 0 if unavailable).
 *
 * @return Heart rate.
 */
int health_get_current_hr(void);

/**
 * @brief Today's step count (or 0 if unavailable).
 *
 * @return Step count.
 */
int health_get_today_steps(void);

/**
 * @brief Today's walked distance in meters (or 0 if unavailable).
 *
 * @return Distance in meters.
 */
int health_get_today_distance(void);

/** @} */
