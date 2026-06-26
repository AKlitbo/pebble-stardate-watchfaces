/**
 * @file settings_schema.h
 * @brief ide-vscode-emery settings schema.
 *
 * A brand-new face that ships its fields from day one, so it starts clean at
 * version 1 with no migration history.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include "settings/settings.h"

/** @addtogroup watchface-vscode @{ */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *vscode_settings_schema(void);

/** @} */
