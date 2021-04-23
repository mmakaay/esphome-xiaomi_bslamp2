# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Changed
- A fix has been implemented to prevent the lamp from being disconnected from Home Assistant
  a lot (resulting in the lamp becoming unavailable).
  Fixes [issue #19](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/19).
  ESPHome v?.?.? or higher is required for making this fix work.
- Transitions are now fully suppressed when in night light mode. In that mode, LED output
  levels are too low for producing good looking transitions.
  Fixes [issue #10](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/10).
- Formatted the codebase using the ESPHome clang rules.

## [1.0.0-RC1]
### Added
- Support for configuring light presets, making it possible to mimic the lamp's original firmware
  feature to switch light color using the color button.
  This fixes [issue #8](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/8).
- New documentation section: Configuration guide (explaining all the YAML file configuration options).
- The documentation has been restructured.

## [1.0-beta]
### Changed
- The component was renamed from "yeelight_bs2" to "xiaomi_bslamp2".
  The reasoning behind this can be found in [issue #7](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/7).

### Added
- Component "text_sensor" that publishes changes in the light mode ("off", "night", "rgb", "white"),
  This fixes [issue #6](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/6).
- Documentation improvements and a better example.yaml.

## [1.0-alpha]
### Added
- All light modes for the lamp have been implemented:
    - RGB light (colored light, based on red/green/blue and brightness)
    - White light (based on color temperature and brightness)
    - Night light (either RGB- or White light, but highly dimmed; enabled by setting brightness to 1%)
- Component "light" for controlling the LEDs.
- Component "binary_sensor" that act as touch/release sensors for power button, color button and slider.
- Component "sensor" that report the level at which the slider was touched.
- Component "output" for controlling the front panel light and its level indicator.

