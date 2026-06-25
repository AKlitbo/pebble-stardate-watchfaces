/**
 * @file settings_schema.h
 * @brief lcars-stardate-emery settings schema: version 2, with a v0 migration that lifts
 * its pre-versioning blob. key 5 and the v1 size are frozen - changing either makes
 * watches in the field lose their settings.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include "settings/settings.h"

/** @addtogroup watchface-lcars @{ */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *lcars_settings_schema(void);

/** @} */
