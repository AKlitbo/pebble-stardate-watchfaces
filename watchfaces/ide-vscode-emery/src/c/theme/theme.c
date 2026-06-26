/**
 * @file theme.c
 * @brief Per-theme lookups: background resource ids and editor text colors.
 *
 * The colors approximate VS Code's syntax palette within the Pebble-64 gamut:
 * keyword-blue for the clock, string-orange/red for the date, mint for the temp +
 * battery, and a light-blue for the steps. Light-theme colors are kept dark so they
 * read on the white editor field; Terminal is a single phosphor green.
 */
#include "theme.h"

uint32_t bg_resource_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:
            return RESOURCE_ID_IMAGE_BG_LIGHT;
        case 2:
            return RESOURCE_ID_IMAGE_BG_TERMINAL;
        case 3:
            return RESOURCE_ID_IMAGE_BG_CYBERPUNK;
        case 4:
            return RESOURCE_ID_IMAGE_BG_SYNTHWAVE;
        case 5:
            return RESOURCE_ID_IMAGE_BG_MONO;
        default:
            return RESOURCE_ID_IMAGE_BG_DARK;
    }
}

GColor time_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Light: VS Code blue (#007ACC)
            return GColorBlueMoon;
        case 2:  // Terminal: phosphor green
            return GColorGreen;
        case 3:  // Cyberpunk: scarlet (#ff0055)
            return GColorFolly;
        case 4:  // Synthwave: neon pink (#ff7edb)
            return GColorShockingPink;
        case 5:  // Mono: white
            return GColorWhite;
        default: // Dark: keyword blue (#569CD6)
            return GColorVividCerulean;
    }
}

GColor date_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Light: dark red string (#A31515)
            return GColorDarkCandyAppleRed;
        case 2:  // Terminal: phosphor green
            return GColorGreen;
        case 3:  // Cyberpunk: function cyan (#00b0ff)
            return GColorVividCerulean;
        case 4:  // Synthwave: keyword yellow (#fede5d)
            return GColorIcterine;
        case 5:  // Mono: gray
            return GColorLightGray;
        default: // Dark: string orange (#CE9178)
            return GColorRajah;
    }
}

GColor stats_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Light: dark green (#008000)
            return GColorDarkGreen;
        case 2:  // Terminal: phosphor green
            return GColorGreen;
        case 3:  // Cyberpunk: number yellow (#fffc58)
            return GColorIcterine;
        case 4:  // Synthwave: function cyan (#36f9f6)
            return GColorElectricBlue;
        case 5:  // Mono: gray
            return GColorLightGray;
        default: // Dark: mint (#4EC9B0)
            return GColorMediumAquamarine;
    }
}

GColor steps_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Light: blue (#0000FF)
            return GColorBlue;
        case 2:  // Terminal: phosphor green
            return GColorGreen;
        case 3:  // Cyberpunk: keyword magenta (#d57bff)
            return GColorShockingPink;
        case 4:  // Synthwave: string orange (#ff8b39)
            return GColorRajah;
        case 5:  // Mono: white
            return GColorWhite;
        default: // Dark: light blue (#9CDCFE)
            return GColorCeleste;
    }
}

GColor hr_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Light: dark red so it reads on white (#AA0000)
            return GColorDarkCandyAppleRed;
        case 2:  // Terminal: phosphor green
            return GColorGreen;
        case 3:  // Cyberpunk: scarlet (#ff0055)
            return GColorFolly;
        case 4:  // Synthwave: scarlet (#ff0055)
            return GColorFolly;
        case 5:  // Mono: white
            return GColorWhite;
        default: // Dark: red bpm
            return GColorRed;
    }
}
