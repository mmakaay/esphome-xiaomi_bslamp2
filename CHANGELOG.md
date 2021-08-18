# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2021.8.0]

**Note**: This release requires ESPHome 2021.8.0 and Home Assistant 2021.8.0 or newer.

### Added
- Preset identifiers (`group` and `preset`) for the `preset.activate` action are now
  validated at compile time. This prevents us from building a firmware with incorrect
  preset identifiers. Before this change, using an invalid preset name would only
  result in a warning message in the device log, which is only moderately useful.

### Changed
- The code has been made compatible with the new color mode support in Home Assistant
  and ESPHome. 
- The `example.yaml` has been updated to not make use of underscores in hostnames.
  Using an underscore in the name yields a warning during the firmware compilation,
  because hostnames should only contain letters, numbers and dashes "-".
- My project will follow the Home Assistant / ESPHome versioning scheme from now on
  (<year>.<month>.<patch>), because the ESPHome project adopted this versioning
  scheme too.

## [1.1.0]

**Note**: This release requires ESPHome v1.20.0 or newer.

### Added
- It is now possible to address the LEDs in the front panel of the device individually.
  There are 12 LEDs in total: the power button, the color button and 10 LEDs that are
  used by the original firmware to represent the lamp's current brightness setting.
  The `output` component for the lamp was updated to provide access to the individual LEDs. 
  Check out the [documentation guide](https://github.com/mmakaay/esphome-xiaomi_bslamp2/blob/main/doc/configuration.md) 
  for details on how to control these.
  Thanks to @Stewie3112 for the feature request that triggered this development!
- Implemented support for visual feedback during the OTA update process in the
  `example.yaml` file: the light becomes blue during flahsing, the brightness slider
  represents the progress, on failure the light flashes red and on success the
  light flashes green.

### Changed
- Made it possible to use lambdas with the `preset.activate` automation. This makes it
  possible to link the action to an api service, which exposes the preset functionality
  to Home Assistant. The `example.yaml` has been updated with an example for this.
- Fixed a rounding error in the slider sensor component. When using custom "range from" / "range to"
  settings, the maximum value could exceed the "range to" value due to rounding errors.
  Thanks to Jos for the heads up!
- Made the codebase compatible with ESPHome v1.19.0
  ([PR #1657: Introduce new async-def coroutine syntax](https://github.com/esphome/esphome/pull/1657))
  Thanks to @Kaibob2 for giving me a heads up that my code was not compiling anymore!

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

