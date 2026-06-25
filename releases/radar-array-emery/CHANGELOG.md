# Changelog - Radar Array Emery

All notable changes to the Stardate Emery watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.2] - 2026-06-24

### Fixed

- Weather provider server errors and rate-limiting are handled gracefully to prevent stale data.
- Out-of-order location search suggestions no longer cause the wrong city to be saved.
- Saving non-weather settings (like theme or date format) no longer triggers a redundant weather fetch.

## [1.0.1] - 2026-06-24

### Fixed

- Unrecognized weather conditions now default to a neutral weather icon.
- Switching to or from .beats time format no longer pauses weather background refreshes.

## [1.0.0] - 2026-06-??
