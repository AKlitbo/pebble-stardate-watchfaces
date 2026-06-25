/**
 * @file zone.h
 * @brief layout primitives: defines paintable areas and text alignments
 *
 * @ingroup lib
 */
// a Zone is everything needed to present one text slot: where it sits, which
// registered font + alignment + colour to use, and up to two smaller font/rect
// tiers to step down to when the text would overflow. A face declares a static
// const table of these; the shell loops it to build the text layers
#pragma once
#include <pebble.h>

#include "fonts.h"

/** @addtogroup lib @{ */

/**
 * @brief The slot vocabulary shared by the current (vitals) face family.
 *
 * A face that diverges from this set would define its own enum + shell.
 */
typedef enum
{
    ZONE_TIME,
    ZONE_MERIDIEM,
    ZONE_DATE,
    ZONE_WEATHER,
    ZONE_COND,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_LAT,
    ZONE_LON,
    ZONE_COUNT
} ZoneId;

/**
 * @brief Defines a paintable area, including geometry and styling.
 */
typedef struct
{
    GRect          rect;
    FontId         font_id;
    GTextAlignment align;
    GColor         color;
    FontId         font_id_fallback;  // first step-down when the text overflows the primary
    GRect          rect_fallback;
    FontId         font_id_fallback2; // second step-down for the widest strings
    GRect          rect_fallback2;    // omit a tier by leaving its rect zero-sized
} Zone;

/**
 * @brief Create transparent text layer for a zone and attach to parent.
 *
 * @param parent The parent layer.
 * @param zone The zone properties.
 * @return The new text layer.
 */
TextLayer *zone_make_layer(Layer *parent, const Zone *zone);

/**
 * @brief Set text, shrinking to fallback if overflow.
 *
 * @param layer The text layer.
 * @param zone The zone specifying the fallback font/rect.
 * @param text The text to set.
 */
void zone_set_text_fit(TextLayer *layer, const Zone *zone, const char *text);

/** @} */
