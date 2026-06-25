/**
 * @file setting_values.h
 * @brief Named values for the multi-choice settings.
 *
 * The wire format and Clay config encode these as bare integers; C branches read
 * them by name instead. The order mirrors each setting's config.js option list,
 * and the trailing *_COUNT sentinel feeds the schema's enum_count so the bound
 * can't drift from the values.
 *
 * @ingroup lib
 */
#pragma once

/** @addtogroup lib @{ */

/**
 * @brief SETTING_TIME_FORMAT choices.
 */
typedef enum
{
    TIME_FORMAT_SYSTEM = 0,
    TIME_FORMAT_12H    = 1,
    TIME_FORMAT_24H    = 2,
    TIME_FORMAT_BEATS  = 3,
    TIME_FORMAT_COUNT
} TimeFormat;

/**
 * @brief SETTING_STEPS_MODE choices.
 */
typedef enum
{
    STEPS_MODE_STEPS = 0,
    STEPS_MODE_MILES = 1,
    STEPS_MODE_KM    = 2,
    STEPS_MODE_COUNT
} StepsMode;

/**
 * @brief Bluetooth connect/disconnect vibe pattern choices.
 */
typedef enum
{
    VIBE_NONE   = 0,
    VIBE_SHORT  = 1,
    VIBE_LONG   = 2,
    VIBE_DOUBLE = 3,
    VIBE_COUNT
} VibeChoice;

/** @} */
