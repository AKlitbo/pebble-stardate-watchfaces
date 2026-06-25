/**
 * @file settings_schema.h
 * @brief radar-array-emery settings schema.
 *
 * A brand-new face that ships the bluetooth fields from day one, so it starts
 * clean at version 1 with no migration history.
 *
 * @ingroup watchface-radar
 */
#pragma once
#include "settings/settings.h"

/** @addtogroup watchface-radar @{ */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *radar_settings_schema(void);

/** @} */
