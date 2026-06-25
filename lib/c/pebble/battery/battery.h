/**
 * @file battery.h
 * @brief battery service: tracks charge level and charging state
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Fires on every battery change, and once at init to seed state.
 *
 * @param state The new battery state.
 */
typedef void (*BatteryChangedHandler)(BatteryChargeState state);

/**
 * @brief Subscribe to battery changes and seed current state.
 *
 * @param handler The callback function.
 */
void battery_init(BatteryChangedHandler handler);

/**
 * @brief Stop tracking battery changes.
 */
void battery_deinit(void);

/**
 * @brief Current charge level and charging state.
 *
 * @return The current battery charge state.
 */
BatteryChargeState battery_peek(void);

/** @} */
