/**
 * @file layout.h
 * @brief Spatial map for the VS Code frame (200x228), kept in sync with the baked background image.
 *
 * The menu bar, activity bar, editor tab, status bar, and the stat glyphs are all
 * baked into the background PNG. The C code paints only the live readouts: the
 * clock (hero), the terminal date/temp/steps values, and the battery percentage.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "shell/shell.h"

/**
 * @defgroup watchface-vscode VS Code Watchface
 * @brief The VS Code Emery watchface.
 * @{
 */

// the editor pane sits below the 16px menu bar and the tab strip, right of the
// 34px activity bar, and above the 20px status bar. the hero clock fills the upper
// editor; a baked "terminal panel" (y150..208) prints the date/temp/steps values

// --- Clock + date ---
// the big single-line clock sits high in the editor with the date right beneath it
// (both above the terminal panel). the date starts 5px below the clock zone.
// zone_set_text_fit applies the font + rect each render
#define SLOT_TIME GRect(34, 21, 166, 84)    // hero clock, raised toward the tab strip
#define SLOT_DATE GRect(34, 110, 166, 22)   // date line, 5px below the clock zone, centered

// --- Terminal readouts (2x2 grid under the prompt) ---
// left column: weather condition over temp; right column: heart rate over steps. each
// value sits just right of its baked glyph
#define SLOT_COND GRect(52, 176, 60, 18)   // top-left, right of the weather glyph
#define SLOT_TEMP GRect(52, 189, 54, 18)   // bottom-left, right of the thermometer glyph
#define SLOT_HR GRect(145, 176, 55, 18)    // top-right, right of the heart glyph (indented 5px)
#define SLOT_STEPS GRect(145, 189, 55, 18) // bottom-right, right of the walking glyph (indented 5px)

// --- Weather glyph ---
// the condition icon is app-drawn (swapped per condition + tinted), sitting just
// left of the condition text in the terminal's top-left stat; 12x12 wi-*-sm master.
// x matches the baked temp-glyph box center so the left column shares a centerline
#define WX_ICON GRect(40, 176, 12, 12)

// --- Battery Readout ---
// both the percentage and the icon are widget-drawn (no shell zone). the percentage
// is right-aligned so it ends just left of the app-drawn battery icon in the tab-strip
// corner; the icon fills to the charge level. either can be hidden via Battery Display
#define BATT_RECT GRect(145, 19, 32, 18)       // percentage, ends just left of the icon
#define BATT_RECT_WIDE GRect(145, 19, 49, 18)   // percentage-only: ends where the icon was
#define BATT_ICON GRect(181, 24, 13, 8)

// --- Bluetooth Glyph ---
// the connection icon is widget-drawn over the baked status bar at the bottom-left,
// where the baked "main ..." text leaves a gap; 10x10 to match the -sm icon
#define BT_ICON GRect(4, 213, 10, 10)

/**
 * @brief The ide-vscode-emery face descriptor (zone table + frame/overlay hooks) for shell_init.
 */
const WatchfaceDescriptor *vscode_emery_face(void);

/** @} */
