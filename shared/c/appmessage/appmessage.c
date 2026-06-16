// appmessage transport: weather requests out, settings + readings in. Decodes
// the wire format and hands results to the app through handlers, so it never
// reaches into the ui shell
#include "appmessage.h"

#include "settings/settings.h"

static AppMessageHandlers s_handlers;

// ask the phone for a fresh weather reading
void appmessage_request_weather(void)
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    app_message_outbox_send();
}

// send the current settings to the phone so Clay can seed its store from the watch
// (the watch persist is the source of truth, the phone's clay-settings can go stale)
static void prv_send_settings(void)
{
    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) != APP_MSG_OK)
    {
        return;
    }

    // selects round-trip as strings, so send Theme/Traversal/Time as cstrings
    char theme_buf[4];
    char traversal_buf[4];
    char time_buf[4];
    snprintf(theme_buf, sizeof(theme_buf), "%d", g_settings.Theme);
    snprintf(traversal_buf, sizeof(traversal_buf), "%d", g_settings.TraversalMode);
    snprintf(time_buf, sizeof(time_buf), "%d", g_settings.TimeFormat);

    dict_write_uint8(iter, MESSAGE_KEY_TEMPERATURE_UNIT, g_settings.TemperatureUnit ? 1 : 0);
    dict_write_cstring(iter, MESSAGE_KEY_DATE_FORMAT, g_settings.DateFormat);
    dict_write_cstring(iter, MESSAGE_KEY_THEME, theme_buf);
    dict_write_cstring(iter, MESSAGE_KEY_TRAVERSAL_MODE, traversal_buf);
    dict_write_cstring(iter, MESSAGE_KEY_TIME_FORMAT, time_buf);

    app_message_outbox_send();
}

// apply an inbox message: weather, coords, and any changed settings
static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
    // the phone asks for the watch's settings on launch to seed Clay, so reply and stop
    if (dict_find(iterator, MESSAGE_KEY_REQUEST_SETTINGS))
    {
        prv_send_settings();
        return;
    }

    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    Tuple *wx_ok_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_OK);

    if (temp_tuple && conditions_tuple)
    {
        // ok flag absent (older JS) is treated as a real reading
        bool wx_ok = (!wx_ok_tuple) || (wx_ok_tuple->value->int32 == 1);
        static char temperature_buffer[8];
        static char conditions_buffer[32];

        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
            conditions_tuple->value->cstring);

        if (wx_ok)
        {
            int temp_value = (int)temp_tuple->value->int32;
            snprintf(temperature_buffer, sizeof(temperature_buffer),
                g_settings.TemperatureUnit ? "%d°F" : "%d°C", temp_value);

            if (s_handlers.on_weather)
            {
                s_handlers.on_weather(temperature_buffer, conditions_buffer);
            }
        }
        else
        {
            // no live reading (e.g. no location set, network error): clear placeholder
            APP_LOG(APP_LOG_LEVEL_INFO, "Weather Unavailable: %s", conditions_buffer);
            if (s_handlers.on_weather)
            {
                s_handlers.on_weather("--", NULL);
            }
        }
    }

    // coordinates arrive pre-formatted as LCARS dash strings ("33-44", "-112-07")
    Tuple *lat_t = dict_find(iterator, MESSAGE_KEY_LATITUDE);
    Tuple *lon_t = dict_find(iterator, MESSAGE_KEY_LONGITUDE);
    if ((lat_t || lon_t) && s_handlers.on_coords)
    {
        s_handlers.on_coords(lat_t ? lat_t->value->cstring : "", lon_t ? lon_t->value->cstring : "");
    }

    Tuple *temp_unit_t = dict_find(iterator, MESSAGE_KEY_TEMPERATURE_UNIT);
    if (temp_unit_t)
    {
        g_settings.TemperatureUnit = temp_unit_t->value->int32 == 1;
    }

    // Clay sends <select> values as strings (HTML <option> values are strings),
    // so the numeric Theme select arrives as a cstring - atoi it. Toggles
    // (TemperatureUnit) arrive as ints, so those are read directly
    Tuple *date_fmt_t = dict_find(iterator, MESSAGE_KEY_DATE_FORMAT);
    if (date_fmt_t && date_fmt_t->value->cstring[0] != '\0')
    {
        strncpy(g_settings.DateFormat, date_fmt_t->value->cstring,
                sizeof(g_settings.DateFormat) - 1);

        g_settings.DateFormat[sizeof(g_settings.DateFormat) - 1] = '\0';
    }

    Tuple *theme_t = dict_find(iterator, MESSAGE_KEY_THEME);
    if (theme_t)
    {
        g_settings.Theme = (uint8_t)atoi(theme_t->value->cstring);
    }

    // TraversalMode and TimeFormat are <select>s, so they arrive as cstrings 
    // and need atoi like Theme above
    Tuple *traversal_t = dict_find(iterator, MESSAGE_KEY_TRAVERSAL_MODE);
    if (traversal_t)
    {
        g_settings.TraversalMode = (uint8_t)atoi(traversal_t->value->cstring);
    }

    Tuple *time_fmt_t = dict_find(iterator, MESSAGE_KEY_TIME_FORMAT);
    if (time_fmt_t)
    {
        g_settings.TimeFormat = (uint8_t)atoi(time_fmt_t->value->cstring);
    }

    if (temp_unit_t || date_fmt_t || theme_t || traversal_t || time_fmt_t)
    {
        settings_save();

        if (s_handlers.on_settings_changed)
        {
            s_handlers.on_settings_changed(date_fmt_t || time_fmt_t);
        }

        if (temp_unit_t)
        {
            appmessage_request_weather();
        }
    }
}

// log a dropped inbox message
static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}

// store the handlers, register callbacks, and open the inbox/outbox
void appmessage_init(AppMessageHandlers handlers)
{
    s_handlers = handlers;
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_open(256, 256);
}
