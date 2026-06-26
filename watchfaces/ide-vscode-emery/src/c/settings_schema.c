/**
 * @file settings_schema.c
 * @brief ide-vscode-emery settings schema.
 *
 * This face ships its fields from day one, so it owns version 1 with no legacy
 * migration. It keeps its own struct and persist key, so its blob and versioning
 * are independent of any other face.
 *
 * @ingroup watchface-vscode
 */
#include "settings_schema.h"
#include "settings/settings_catalog.h"
#include "settings/setting_values.h"

#include <stddef.h>

#define VSCODE_SETTINGS_KEY 1
#define VSCODE_SETTINGS_VERSION 1

/** @addtogroup watchface-vscode @{ */

/**
 * @brief The full schema for this face.
 */
typedef struct VscodeSettings
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
    uint8_t battery_display;
} VscodeSettings;

static VscodeSettings s_settings;

// the date defaults to a compact single line ("JUN 18") to fit the terminal panel.
// 6 themes: Dark+, Light, Terminal, Cyberpunk, Synthwave, Mono.
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(VscodeSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(VscodeSettings, date_format), "%b %d"),
    KNOWN_THEME(offsetof(VscodeSettings, theme), 6),
    KNOWN_STEPS_MODE(offsetof(VscodeSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(VscodeSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(VscodeSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(VscodeSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(VscodeSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_BATTERY_DISPLAY(offsetof(VscodeSettings, battery_display), BATTERY_DISPLAY_COUNT),
};

static const SettingsSchema s_schema = {
    .key = VSCODE_SETTINGS_KEY,
    .version = VSCODE_SETTINGS_VERSION,
    // unshipped fields all landed in v1, so the current struct is the v1 size
    .min_versioned_size = sizeof(VscodeSettings),
    .blob = &s_settings,
    .blob_size = sizeof(VscodeSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // no legacy blobs - this face never existed without these fields
};

const SettingsSchema *vscode_settings_schema(void)
{
    return &s_schema;
}

/** @} */
