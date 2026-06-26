# Pebble Watchfaces

A collection of watchfaces for Pebble smartwatches. These watchfaces display the time, date, weather, heart rate, step count, and battery status, wrapped in various stylized interfaces like LCARS and radar arrays.

## Project Structure

This repository contains multiple watchface variants and the shared library that powers them:

* **`watchfaces/`**: Individual watchface projects.
  * `lcars-stardate-emery`: A standard LCARS themed watchface for the Pebble Time 2 (**Emery**).
  * `radar-array-emery`: A radar-array themed watchface for the Pebble Time 2 (**Emery**).
  * `ide-vscode-emery`: A VS Code / code-editor themed watchface for the Pebble Time 2 (**Emery**).
* **`lib/`**: The shared engine powering all watchfaces.
  * **`c/` & `js/`**: Common device/host code (weather providers, unit conversion, health data, UI components).
  * **`css/`**: Shared CSS stylesheets used across web-based configuration pages and preview frames.
  * **`py/`**: Centralized Python helpers that abstract away the Pebble SDK build boilerplate for all watchfaces.
  * **`resources/`**: Common image assets and icons shared between faces (e.g., weather condition glyphs, connection symbols).
* **`tools/`**: Shared Node.js development utilities for rendering watchface preview frames, rasterizing SVG icons, and generating C resource tables.
* **`vendor/`**: Third-party assets and templates.

## Watchfaces

| Watchface | Preview |
| :--- | :--- |
| **LCARS Stardate** | ![Classic][lcars-1] ![Nemesis][lcars-7] ![Mono][lcars-2] ![Lower Decks][lcars-3] ![Lower Decks Mono][lcars-4] ![PADD][lcars-5] ![PADD Mono][lcars-6] |
| **Radar Array** | ![Default][radar-1] ![Crimson][radar-2] ![Neon][radar-3] ![Phosphor][radar-4] ![Rescue][radar-5] ![Stealth][radar-6] ![Mono][radar-7] |
| **IDE / VS Code** | ![Dark][ide-1] ![Light][ide-2] ![Terminal][ide-3] ![Cyberpunk][ide-4] ![Synthwave][ide-5] ![Mono][ide-6] |

## Credits

* **LCARS Stardate**
  * **LCARS Design**: LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications.
  * **Typography**: [Antonio](https://fonts.google.com/specimen/Antonio).
  * **Weather Icons**: [Erik Flowers](https://github.com/erikflowers/weather-icons).
  * **Glyphs**: Heart, step, and thermometer icons from [UXWing](https://uxwing.com).

* **Radar Array**
  * **Typography**: [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).

* **IDE / VS Code**
  * **Typography**: [Teko](https://fonts.google.com/specimen/Teko) and [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).

* **General**
  * **Bluetooth Icons**: Bluetooth on / slash icons from [SVG Repo](https://www.svgrepo.com).
  * **Built With**: [Pebble SDK](https://developer.repebble.com) and [Clay](https://github.com/pebble-dev/clay).

## Third-Party Assets

This repository bundles the Antonio font and generated icon PNGs (rasterized from vendored SVG sources). The Weather Icons, UXWing SVGs, and the TheLCARS.com frame template are *not* bundled and must be fetched independently to regenerate icons or bake frames; the SVG Repo Bluetooth SVGs are bundled under `vendor/svgrepo/`. They remain under their respective open-source licenses or terms:

**LCARS Stardate**
* [Antonio Font](https://fonts.google.com/specimen/Antonio) (SIL Open Font License 1.1), bundled
* [Weather Icons by Erik Flowers](https://github.com/erikflowers/weather-icons) (SIL Open Font License 1.1 / MIT), rasterized from SVG sources, rendered PNGs bundled
* [UXWing Icons](https://uxwing.com) (UXWing License), SVG sources fetched separately, rendered PNGs bundled
* LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications, fetched separately, not bundled

**Radar Array**
* [Share Tech Mono Font](https://fonts.google.com/specimen/Share+Tech+Mono) (SIL Open Font License 1.1), bundled

**IDE / VS Code**
* [Teko Font](https://fonts.google.com/specimen/Teko) (SIL Open Font License 1.1), bundled
* [Share Tech Mono Font](https://fonts.google.com/specimen/Share+Tech+Mono) (SIL Open Font License 1.1), bundled

**General**
* [Bluetooth Icons from SVG Repo](https://www.svgrepo.com) ([CC Attribution](https://www.svgrepo.com/page/licensing/#CC%20Attribution)), SVG sources bundled under `vendor/svgrepo/`, rendered PNGs bundled

Please refer to their original source repositories and websites for full license and usage details.

## License

**Source Code:** © 2026 Andrew Klitbo (Null Syntax), licensed under the [PolyForm Noncommercial License 1.0.0](LICENSE). This license was chosen to help keep the project aligned with the non-commercial nature of the LCARS-inspired assets and Star Trek fan-project guidelines.

You may use, modify, fork, and share it freely for any **noncommercial** purpose, personal use, hobby projects, study, and the like. See [LICENSE](LICENSE) for the full terms.

## Disclaimer

**LCARS Stardate** is a non-commercial fan project. *Star Trek*, LCARS, and related marks are trademarks of CBS / Paramount Global. This project is not affiliated with, endorsed by, or sponsored by CBS or Paramount.

## AI Training Notice

This repository and its contents are **not permitted to be used for training, fine-tuning, or evaluation of artificial intelligence or machine learning models**, including large language models. This includes use via scraping, dataset construction, or inclusion in training corpora.

No consent is granted for such use.

<!-- Image Ref -->
[lcars-1]: releases/lcars-stardate-emery/store/theme_classic.png "Classic"
[lcars-2]: releases/lcars-stardate-emery/store/theme_mono.png "Classic Mono"
[lcars-3]: releases/lcars-stardate-emery/store/theme_lower-decks.png "Lower Decks"
[lcars-4]: releases/lcars-stardate-emery/store/theme_lower-decks-mono.png "Lower Decks Mono"
[lcars-5]: releases/lcars-stardate-emery/store/theme_lower-decks-padd.png "Lower Decks PADD"
[lcars-6]: releases/lcars-stardate-emery/store/theme_lower-decks-padd-mono.png "Lower Decks PADD Mono"
[lcars-7]: releases/lcars-stardate-emery/store/theme_nemesis-blue.png "Nemesis Blue"

[radar-1]: releases/radar-array-emery/store/theme_default.png "Default"
[radar-2]: releases/radar-array-emery/store/theme_crimson.png "Crimson"
[radar-3]: releases/radar-array-emery/store/theme_neon.png "Neon"
[radar-4]: releases/radar-array-emery/store/theme_phosphor.png "Phosphor"
[radar-5]: releases/radar-array-emery/store/theme_rescue.png "Rescue"
[radar-6]: releases/radar-array-emery/store/theme_stealth.png "Stealth"
[radar-7]: releases/radar-array-emery/store/theme_mono.png "Mono"

[ide-1]: releases/ide-vscode-emery/store/theme_dark.png "Dark"
[ide-2]: releases/ide-vscode-emery/store/theme_light.png "Light"
[ide-3]: releases/ide-vscode-emery/store/theme_terminal.png "Terminal"
[ide-4]: releases/ide-vscode-emery/store/theme_cyberpunk.png "Cyberpunk"
[ide-5]: releases/ide-vscode-emery/store/theme_synthwave.png "Synthwave"
[ide-6]: releases/ide-vscode-emery/store/theme_mono.png "Mono"