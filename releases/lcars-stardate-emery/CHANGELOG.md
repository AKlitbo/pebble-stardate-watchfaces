# Changelog - LCARS Stardate Emery

All notable changes to the Stardate Emery watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.1] - 2026-06-24

### Fixed

- Unrecognized weather conditions now fall back to a neutral icon instead of sometimes showing the wrong one. Longer descriptions from some weather providers were previously cut short, which could match the wrong icon or none at all.
- Weather keeps refreshing on its normal half-hour schedule when you switch the time format to or from .beats. Changing modes could previously leave the reading stale for up to half an hour.

## [1.2.0] - 2026-06-20

### Added

- Added a Bluetooth connection icon that shows whether the watch is linked to your phone. It is shown by default and can be toggled off in settings.
- Added optional connection alerts that buzz the watch when the phone reconnects or disconnects. Each can be set independently to None, Short, Long, or Double. Both are off by default and can be enabled in settings.

### Fixed

- The longest date formats now shrink to fit within the date banner instead of being clipped. Wide layouts such as the text month and day-of-year formats previously overflowed the banner, and the smaller fallback sizes now sit centered within it.

## [1.1.0] - 2026-06-18

### Added

- Added many additional date format options, including day-first, month-first, and year-first layouts, text month formats, and two day-of-year formats.

## [1.0.3] - 2026-06-17

### Fixed

- Swatch .beats now updates the instant a beat rolls over. It previously refreshed only on the minute tick, causing the @beat readout to lag behind.

### Changed

- The date line now updates only when it changes, reducing unnecessary redraws.

## [1.0.2] - 2026-06-17

### Fixed

- The battery gauge is now legible on the Lower Decks PADD theme. Its dark-blue fill was indistinguishable from the empty segments, so the charge level could not be read.

### Changed

- The battery gauge now uses a consistent style across all themes. Charged cells are filled with the theme's accent color, while empty cells are outlined in the same color.

## [1.0.1] - 2026-06-16

> [!IMPORTANT]
> Due to a build mix-up, the 1.0.0 app store release was not built from the initial commit on main. This changelog tracks changes against the main branch history, so some entries below may not match the exact contents of the published 1.0.0 binary.

### Fixed

- Coordinate readout no longer shows an incorrect final digit. The fractional portion is now rounded instead of truncated.
- Saved settings now survive app updates. Stored settings are versioned and migrated across versions, and anything unreadable falls back to safe defaults, preventing blank date lines or incorrect clock formats.
- The settings screen now opens with the current saved values instead of defaults. The watch sends its active configuration to the phone when the app launches, so saving no longer overwrites existing choices.
- Weather icons now reload only when the condition actually changes, avoiding a redundant bitmap reload on every refresh.
- The weather condition readout no longer clips "CLEAR" to "CLEA..." at night. It now shows "CLEAR" while still using the night (moon) icon.

### Changed

- Traversal distance is now rounded to the nearest tenth instead of truncated. For example, 4.96 km now displays as 5.0 KM instead of 4.9 KM.
- Health stats (heart rate, steps) are now refreshed only when new readings are reported, reducing battery usage.

## [1.0.0] - 2026-06-14
