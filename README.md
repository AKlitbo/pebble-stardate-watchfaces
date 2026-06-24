# Pebble Watchfaces

A collection of watchfaces for Pebble smartwatches. These watchfaces display the time, date, weather, heart rate, step count, and battery status, wrapped in various stylized interfaces like LCARS and radar arrays.

## Project Structure

This repository contains multiple watchface variants and the shared library that powers them:

* **`watchfaces/`**: Individual watchface projects.
  * `lcars-stardate-emery`: A standard LCARS themed watchface for the Pebble Time 2 (**Emery**).
  * `radar-array-emery`: A radar-array themed watchface for the Pebble Time 2 (**Emery**).
* **`lib/`**: Common C and JavaScript code used across all watchfaces, including weather providers, unit conversion, health data, and shell components.
* **`tools/`**: Development utilities for rasterizing icons from SVG sources. 
* **`vendor/`**: Third-party assets and templates.

## Screenshots

| Watchface | Preview |
| :--- | :--- |
| **LCARS Stardate** | <img src="releases/lcars-stardate-emery/store/theme_classic.png" width="144" title="Classic" /> <img src="releases/lcars-stardate-emery/store/theme_lower-decks.png" width="144" title="Lower Decks" /> <img src="releases/lcars-stardate-emery/store/theme_lower-decks-padd.png" width="144" title="Lower Decks PADD" /> <img src="releases/lcars-stardate-emery/store/theme_nemesis-blue.png" width="144" title="Nemesis Blue" /> |
| **Radar Array** | <img src="releases/radar-array-emery/store/theme_default.png" width="144" title="Default" /> <img src="releases/radar-array-emery/store/theme_crimson.png" width="144" title="Crimson" /> <img src="releases/radar-array-emery/store/theme_neon.png" width="144" title="Neon" /> <img src="releases/radar-array-emery/store/theme_phosphor.png" width="144" title="Phosphor" /> <img src="releases/radar-array-emery/store/theme_rescue.png" width="144" title="Rescue" /> <img src="releases/radar-array-emery/store/theme_stealth.png" width="144" title="Stealth" /> |

## Credits

* **LCARS Stardate**
  * **LCARS Design**: LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications.
  * **Typography**: [Antonio](https://fonts.google.com/specimen/Antonio).
  * **Weather Icons**: [Erik Flowers](https://github.com/erikflowers/weather-icons).
  * **Glyphs**: Heart, step, and thermometer icons from [UXWing](https://uxwing.com).

* **Radar Array**
  * **Typography**: [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).

* **General**
  * **Bluetooth Icons**: Bluetooth on / slash icons from [SVG Repo](https://www.svgrepo.com).
  * **Built With**: [Pebble SDK](https://developer.repebble.com) and [Clay](https://github.com/pebble-dev/clay).

## Third-Party Assets

This repository bundles the Antonio font and generated icon PNGs under `lib/resources/icons/` (rasterized from vendored SVG sources). The Weather Icons, UXWing SVGs, and the TheLCARS.com frame template are *not* bundled and must be fetched independently to regenerate icons or bake frames; the SVG Repo Bluetooth SVGs are bundled under `vendor/svgrepo/`. They remain under their respective open-source licenses or terms:

**LCARS Stardate**
* [Antonio Font](https://fonts.google.com/specimen/Antonio) (SIL Open Font License 1.1), bundled
* [Weather Icons by Erik Flowers](https://github.com/erikflowers/weather-icons) (SIL Open Font License 1.1 / MIT), rasterized from SVG sources, rendered PNGs bundled
* [UXWing Icons](https://uxwing.com) (UXWing License), SVG sources fetched separately, rendered PNGs bundled
* LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications, fetched separately, not bundled

**Radar Array**
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