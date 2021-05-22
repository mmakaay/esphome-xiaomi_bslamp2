# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Not yet released]

### Changed
- Made the codebase compatible with ESPHome v1.x.0
  ([PR #1657: Introduce new async-def coroutine syntax](https://github.com/esphome/esphome/pull/1657))

### Added
- Implemented support for visual feedback during the OTA flashing process in the
  `example.yaml` file: the light becomes blue during the process, the brightness bar
  represents the update progress, when updating fails the light flashes red and when it
  completes successfuly, the light flashes green.

## [1.0.0]

**Note**: This release requires ESPHome v1.18.0 or newer.

### Changed
- The GitHub repository structure has been updated, in order to make it compatible with
  the new `external_components` feature of ESPHome v1.18.0. From now on, the code from this
  repository no longer has to be downloaded and installed manually. The `example.yaml` has
  been updated for using this new feature.
  Fixes [issue #16](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/16).
- A fix has been implemented to prevent the lamp from being disconnected from Home Assistant
  a lot (resulting in the lamp becoming unavailable). ESPHome v1.18.0 or newer is required
  for making this fix work.
  Fixes [issue #19](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/19).
- Transitions are now fully suppressed when in night light mode. In that mode, LED output
  levels are too low for producing good looking transitions.
  Fixes [issue #10](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/10).
- The `example.yaml` now uses my own arduino-esp32 unicore build.
- Formatted the codebase using the ESPHome clang rules.
- The flashing guide got a big update, with detailed pictures for every step on the way.

### Added
- New documentation section: Technical details (it's all about the internals)

### Removed
- The known issue documentation page has been removed, now we have a fix for the disconnect
  issues in ESPHome v1.18.0.

## [1.0.0-RC1]
### Changed
- The documentation has been restructured.

### Added
- Support for disco mode-style light updates through the `light.disco_on` and `light.disco_off` actions.
  The `light.disco_on` action can be used to configure the lamp state without delay, without publishing
  the update and without saving the state. The `light.disco_off` action will restore the lamp to its
  last save state, so from before the `light.disco_on` updates.
- Support for configuring light presets, making it possible to mimic the lamp's original firmware
  feature to switch light color using the color button.
  This fixes [issue #8](https://github.com/mmakaay/esphome-xiaomi_bslamp2/issues/8).
- New documentation section: Configuration guide (explaining all the YAML file configuration options).

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

