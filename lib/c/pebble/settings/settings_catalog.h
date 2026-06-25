/**
 * @file settings_catalog.h
 * @brief the shared catalog of known settings: a KNOWN_* macro per setting that bakes
 * in its neutral wire key, encoding, bounds, default, and side effects
 *
 * A face subscribes to a known setting by placing its KNOWN_* entry in the face's
 * field table at the offset of the matching struct member - and by declaring the
 * setting's MESSAGE_KEY in the face's package.json. Face-only settings are declared
 * inline in the table instead, with an id of SETTING_COUNT or greater.
 *
 * @ingroup lib
 */
#pragma once
#include "settings/settings.h"

/** @addtogroup lib @{ */

/**
 * @brief Setting definition for Temperature Unit.
 * @param off Offset into the face's settings struct.
 */
#define KNOWN_TEMPERATURE_UNIT(off) \
    { .id = SETTING_TEMPERATURE_UNIT, .message_key = &MESSAGE_KEY_TEMPERATURE_UNIT, \
      .type = SETTING_BOOL, .offset = (off), .affects_weather = true }

/**
 * @brief Setting definition for Date Format.
 * @param off Offset into the face's settings struct.
 * @param dflt The default format string.
 */
#define KNOWN_DATE_FORMAT(off, dflt) \
    { .id = SETTING_DATE_FORMAT, .message_key = &MESSAGE_KEY_DATE_FORMAT, \
      .type = SETTING_CSTRING, .offset = (off), .size = 16, .default_str = (dflt), \
      .affects_layout = true }

/**
 * @brief Setting definition for Theme.
 * @param off Offset into the face's settings struct.
 * @param count The number of enum values.
 */
#define KNOWN_THEME(off, count) \
    { .id = SETTING_THEME, .message_key = &MESSAGE_KEY_THEME, \
      .type = SETTING_ENUM_U8, .offset = (off), .enum_count = (count) }

/**
 * @brief Setting definition for Steps Mode.
 * @param off Offset into the face's settings struct.
 * @param count The number of enum values.
 */
#define KNOWN_STEPS_MODE(off, count) \
    { .id = SETTING_STEPS_MODE, .message_key = &MESSAGE_KEY_STEPS_MODE, \
      .type = SETTING_ENUM_U8, .offset = (off), .enum_count = (count) }

/**
 * @brief Setting definition for Time Format.
 * @param off Offset into the face's settings struct.
 * @param count The number of enum values.
 */
#define KNOWN_TIME_FORMAT(off, count) \
    { .id = SETTING_TIME_FORMAT, .message_key = &MESSAGE_KEY_TIME_FORMAT, \
      .type = SETTING_ENUM_U8, .offset = (off), .enum_count = (count), \
      .affects_layout = true }

/**
 * @brief Setting definition for Bluetooth Icon.
 * @param off Offset into the face's settings struct.
 */
#define KNOWN_BLUETOOTH_ICON(off) \
    { .id = SETTING_BLUETOOTH_ICON, .message_key = &MESSAGE_KEY_BLUETOOTH_ICON, \
      .type = SETTING_BOOL, .offset = (off), .default_num = 1 }

/**
 * @brief Setting definition for Bluetooth Vibe Connect.
 * @param off Offset into the face's settings struct.
 * @param count The number of enum values.
 */
#define KNOWN_BLUETOOTH_VIBE_CONNECT(off, count) \
    { .id = SETTING_BLUETOOTH_VIBE_CONNECT, .message_key = &MESSAGE_KEY_BLUETOOTH_VIBE_CONNECT, \
      .type = SETTING_ENUM_U8, .offset = (off), .enum_count = (count) }

/**
 * @brief Setting definition for Bluetooth Vibe Disconnect.
 * @param off Offset into the face's settings struct.
 * @param count The number of enum values.
 */
#define KNOWN_BLUETOOTH_VIBE_DISCONNECT(off, count) \
    { .id = SETTING_BLUETOOTH_VIBE_DISCONNECT, .message_key = &MESSAGE_KEY_BLUETOOTH_VIBE_DISCONNECT, \
      .type = SETTING_ENUM_U8, .offset = (off), .enum_count = (count) }

/** @} */
