/**
 * @file vibe.h
 * @brief haptic feedback wrapper
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Defines the available haptic pulse patterns.
 */
typedef enum
{
    VibePulseShort,
    VibePulseLong,
    VibePulseDouble,
} VibePulse;

/**
 * @brief Fire a pulse unless in quiet time.
 *
 * @param pulse The pulse pattern to fire.
 */
void vibe_pulse(VibePulse pulse);

/** @} */
