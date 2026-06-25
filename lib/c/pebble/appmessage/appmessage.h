/**
 * @file appmessage.h
 * @brief appmessage transport: decodes the inbox and notifies the app via handlers
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Callbacks the app supplies so the transport stays decoupled from the ui shell.
 */
typedef struct
{
    void (*on_weather)(const char *temp, const char *condition);  /**< Callback for weather updates */
    void (*on_coords)(const char *lat, const char *lon);          /**< Callback for coordinates updates */
    void (*on_settings_changed)(bool time_or_date_changed);       /**< Callback for settings changes */
} AppMessageHandlers;

/**
 * @brief Stores the handlers, registers callbacks, and opens the inbox/outbox.
 *
 * @param handlers The struct containing the callback functions.
 */
void appmessage_init(AppMessageHandlers handlers);

/**
 * @brief Asks the phone for a fresh weather reading.
 */
void appmessage_request_weather(void);

/**
 * @brief Asks for weather only if the refresh interval has elapsed since the last request.
 *
 * Owns the poll cadence so a face can drive it from any ticker (the minute tick
 * or a .beats timer) without tracking the last-poll time itself - the single
 * elapsed-time clock survives swapping tickers, so weather never goes stale.
 *
 * @param now The current wall-clock time.
 */
void appmessage_request_weather_if_due(time_t now);

/** @} */
