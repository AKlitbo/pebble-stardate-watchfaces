/**
 * @file beats.h
 * @brief pure swatch .beats math, operates on ms since Biel Mean Time (UTC+1) midnight
 *
 * @ingroup lib
 */
#pragma once
#include <stdint.h>

// shared time constants for the .beats clock and the BMT conversion
#define MS_PER_BEAT      86400LL     // milliseconds in one .beat (1/1000 of a day)
#define MS_PER_DAY       86400000LL  // milliseconds in a full day
#define MS_PER_SEC       1000LL
#define SECS_PER_DAY     86400
#define SECS_PER_HOUR    3600
#define SECS_PER_MIN     60
#define BMT_UTC_OFFSET_S 3600        // Biel Mean Time = UTC+1

/** @addtogroup lib @{ */

/**
 * @brief Calculates swatch internet time in .beats (0..999) for a given ms.
 *
 * @param ms_into_bmt_day Milliseconds elapsed since Biel Mean Time (UTC+1) midnight.
 * @return The current time in .beats (0 to 999).
 */
int beats_from_ms(int64_t ms_into_bmt_day);

/**
 * @brief Calculates milliseconds until the next .beats boundary.
 *
 * A single beat is exactly 86,400 milliseconds. This is useful for
 * scheduling timers to trigger exactly when the beat rolls over.
 *
 * @param ms_into_bmt_day Milliseconds elapsed since Biel Mean Time (UTC+1) midnight.
 * @return Milliseconds remaining until the next beat boundary (1 to 86400).
 */
uint32_t ms_until_next_beat(int64_t ms_into_bmt_day);

/** @} */
