/**
 * @file connection.h
 * @brief pebble connection (bluetooth) service: tracks whether the phone is in range
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Fires on every phone-app connection change, and once at init to seed state.
 *
 * @param connected True if the phone is connected, false otherwise.
 */
typedef void (*ConnectionChangedHandler)(bool connected);

/**
 * @brief Subscribe to phone-app connection changes and seed current state.
 *
 * @param handler The callback function.
 */
void connection_init(ConnectionChangedHandler handler);

/**
 * @brief Stop tracking connection changes.
 */
void connection_deinit(void);

/**
 * @brief Check if the phone is currently connected.
 *
 * @return True when the phone (pebble app) is currently connected.
 */
bool connection_is_connected(void);

/** @} */
