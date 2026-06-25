/**
 * @file settings_schema.c
 * @brief lcars-stardate-emery settings schema: this face's persisted-settings identity.
 *
 * key 5 and the 21-byte v1 floor are frozen - changing either makes watches in the
 * field lose their settings.
 *
 * @ingroup watchface-lcars
 */
#include "settings_schema.h"
#include "settings/settings_catalog.h"
#include "settings/setting_values.h"

#include <stddef.h>
#include <string.h>

#define LCARS_SETTINGS_KEY 5
#define LCARS_SETTINGS_VERSION 2
// smallest versioned blob accepted; fields are append-only so this never changes
#define LCARS_SETTINGS_V1_SIZE 21

/** @addtogroup watchface-lcars @{ */

/**
 * @brief lcars's persisted settings.
 *
 * The byte layout is frozen: it shipped as a flat blob (version byte, then
 * each field in this order), so members may be renamed but never reordered,
 * retyped, or inserted - only appended. The static asserts below guard it.
 */
typedef struct LcarsSettings
{
    uint8_t version;
    bool    temperature_unit;
    char    date_format[16];
    uint8_t theme;
    uint8_t steps_mode;
    uint8_t time_format;
    bool    bluetooth_icon;
    uint8_t vibe_connect;
    uint8_t vibe_disconnect;
} LcarsSettings;

_Static_assert(sizeof(LcarsSettings) == 24, "lcars blob size is frozen at 24 bytes (v2)");
_Static_assert(offsetof(LcarsSettings, bluetooth_icon) == LCARS_SETTINGS_V1_SIZE,
               "lcars v1 floor is frozen; the v2 fields must stay appended after it");

static LcarsSettings s_settings;

// lcars subscribes to every known setting, in its frozen struct order. "%Y.%m%d" is
// its numeric stardate-style date default.
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(LcarsSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(LcarsSettings, date_format), "%Y.%m%d"),
    KNOWN_THEME(offsetof(LcarsSettings, theme), 4),
    KNOWN_STEPS_MODE(offsetof(LcarsSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(LcarsSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(LcarsSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(LcarsSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(LcarsSettings, vibe_disconnect), VIBE_COUNT),
};

/**
 * @brief The pre-versioning layout (no version field) that shipped under key 5.
 *
 * Kept only so the migration can lift an existing blob into the current struct.
 */
typedef struct ClaySettingsV0
{
    bool TemperatureUnit;
    char DateFormat[16];
    uint8_t Theme;
    uint8_t StepsMode;
    uint8_t TimeFormat;
} ClaySettingsV0;

/**
 * @brief Lift a pre-versioning blob, preserving every field.
 *
 * Recognised purely by its size, which never collides with a versioned blob.
 *
 * @param stored_size The size of the stored blob.
 * @return True if migrated, false otherwise.
 */
static bool lcars_migrate(int stored_size)
{
    if (stored_size != (int)sizeof(ClaySettingsV0))
    {
        return false;
    }

    ClaySettingsV0 legacy;
    persist_read_data(LCARS_SETTINGS_KEY, &legacy, sizeof(legacy));

    s_settings.temperature_unit = legacy.TemperatureUnit;
    memcpy(s_settings.date_format, legacy.DateFormat, sizeof(s_settings.date_format));
    s_settings.theme = legacy.Theme;
    s_settings.steps_mode = legacy.StepsMode;
    s_settings.time_format = legacy.TimeFormat;

    return true;
}

static const SettingsSchema s_schema = {
    .key = LCARS_SETTINGS_KEY,
    .version = LCARS_SETTINGS_VERSION,
    .min_versioned_size = LCARS_SETTINGS_V1_SIZE,
    .blob = &s_settings,
    .blob_size = sizeof(LcarsSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = lcars_migrate,
};

const SettingsSchema *lcars_settings_schema(void)
{
    return &s_schema;
}

/** @} */
