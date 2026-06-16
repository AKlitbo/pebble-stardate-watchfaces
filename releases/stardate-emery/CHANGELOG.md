# Changelog - Stardate Emery

All notable changes to the Stardate Emery watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2026-06-15

> [!IMPORTANT]
> Due to a build mix-up, the 1.0.0 app store release was not built from the initial commit on main. This changelog tracks changes against the main branch history, so some entries below may not match the exact contents of the published 1.0.0 binary.

### Changed

- TRAVERSAL distance is now rounded to the nearest tenth instead of truncated, so an actual distance of 4.96 km now shows as 5.0 KM rather than the old, truncated 4.9 KM.

### Fixed

- Coordinate readout no longer shows an incorrect final digit. The fractional portion is now rounded instead of truncated.
- Saved settings now survive future updates, and a corrupted or mismatched settings store can no longer blank the date line or switch the clock into the wrong time format. Storage is versioned and migrated when possible, and anything unreadable falls back to safe defaults instead of being displayed as garbage.
- Opening settings after an update and saving no longer overwrites your choices with defaults. The watch now shares its current settings with the phone when the app launches, so the configuration screen opens with your real values and saving re-applies them.
- Weather icons now reload only when the condition actually changes, avoiding a redundant bitmap reload on every refresh.
- The weather condition readout no longer clips a clear night to "CLEA...". It now shows "CLEAR" while still using the night (moon) icon.
- Battery life is improved by refreshing health stats (heart rate, steps, distance) only when the watch reports new readings, rather than re-reading them every minute and on every battery change.

## [1.0.0] - 2026-06-14
