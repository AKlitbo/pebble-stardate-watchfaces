/**
 * @file main.c
 * @brief ide-vscode-emery entry point: wires services to the ui shell.
 *
 * Shows the weather condition and temperature in the terminal stats row, so it
 * polls weather every 30 min. It never displays coordinates, so on_coords is
 * left unset and the JS sends none.
 */
#include <pebble.h>

#include "appmessage/appmessage.h"
#include "battery/battery.h"
#include "health/health.h"
#include "layout.h"
#include "settings/settings.h"
#include "settings/setting_values.h"
#include "settings_schema.h"
#include "shell/shell.h"
#include "units/units.h"

/**
 * @brief Pull fresh health readings into the ui (driven by health events, not polled).
 */
static void update_health(void)
{
    int hr = health_get_current_hr();
    int steps = health_get_today_steps();
    int distance_m = health_get_today_distance();

    shell_update_info(hr, steps, distance_m);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

// fires at each .beats boundary so the @beat readout never sits stale between the
// 86.4s beats. NULL when not in .beats mode, where MINUTE_UNIT drives instead
static AppTimer *s_beats_timer;

/**
 * @brief Redraw the time slot and re-arm for the next beat boundary.
 *
 * Also keeps the 30-min weather poll alive, since the minute ticker that
 * owns it is off in .beats mode.
 *
 * @param data The timer context (unused).
 */
static void beats_timer_handler(void *data)
{
    time_t now = time(NULL);

    // real tm: shell_update_time still draws the date line
    shell_update_time(localtime(&now));

    appmessage_request_weather_if_due(now);

    s_beats_timer = app_timer_register(units_ms_until_next_beat(), beats_timer_handler, NULL);
}

/**
 * @brief Run exactly one ticker for the active format.
 *
 * The beats AppTimer in .beats mode, MINUTE_UNIT otherwise. Duplicate calls
 * are harmless, so it's safe to call on init and every settings save.
 */
static void update_refresh_mode(void)
{
    if (settings_u8(SETTING_TIME_FORMAT) == TIME_FORMAT_BEATS)
    {
        if (!s_beats_timer)
        {
            // hand the cadence to the beats timer; the weather poll clock lives in the
            // appmessage module, so it carries over the swap unchanged
            tick_timer_service_unsubscribe();
            s_beats_timer = app_timer_register(units_ms_until_next_beat(), beats_timer_handler, NULL);
        }
    }
    else if (s_beats_timer)
    {
        app_timer_cancel(s_beats_timer);
        s_beats_timer = NULL;

        // resume the minute ticker
        tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
}

/**
 * @brief Redraw after a settings change, re-rendering the clock when the time/date format changed.
 *
 * @param time_or_date_changed True if the time or date format changed.
 */
static void on_settings_changed(bool time_or_date_changed)
{
    shell_update_display();

    if (time_or_date_changed)
    {
        time_t now = time(NULL);
        shell_update_time(localtime(&now));
    }

    // swap to the right ticker if TimeFormat changed
    update_refresh_mode();
}

/**
 * @brief Per-minute: redraw the time, poll weather every 30 min.
 *
 * Health refreshes from health events, not polled here.
 *
 * @param tick_time The current time.
 * @param units_changed The units that changed.
 */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    shell_update_time(tick_time);

    appmessage_request_weather_if_due(time(NULL));
}

/**
 * @brief Refresh health readouts only when the health service reports new data.
 *
 * @param event The health event type.
 * @param context Context (unused).
 */
static void health_handler(HealthEventType event, void *context)
{
    if (event == HealthEventHeartRateUpdate || event == HealthEventMovementUpdate ||
        event == HealthEventSignificantUpdate)
    {
        update_health();
    }
}

/**
 * @brief Redraw the battery gauge when the charge level changes.
 *
 * @param state The new battery state.
 */
static void battery_callback(BatteryChargeState state)
{
    shell_set_battery(state.charge_percent);
}

/**
 * @brief Initialize sub-systems, seed the first render, and subscribe to services.
 */
static void init(void)
{
    settings_init(vscode_settings_schema());
    shell_init(vscode_emery_face());

    // wire the appmessage transport to the shell (the transport stays shell-agnostic).
    // coords aren't displayed, so on_coords stays unset and the JS sends none
    appmessage_init((AppMessageHandlers){
        .on_weather = shell_set_weather,
        .on_coords = NULL,
        .on_settings_changed = on_settings_changed,
    });

    health_init(health_handler);

    // seed the initial time/date render before the first tick
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    shell_update_time(tick_time);

    // services and callbacks
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_init(battery_callback);

    // seed the health readouts before the first health event
    update_health();

    // pick the ticker for the active format (swaps off MINUTE_UNIT if .beats is on)
    update_refresh_mode();
}

/**
 * @brief Tear down the ui shell.
 */
static void deinit(void)
{
    shell_deinit();
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
