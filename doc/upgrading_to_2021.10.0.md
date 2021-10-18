# Upgrading to version 2021.10.0

Quite a few things have been changed in order to make the Xiaomi Bedside Lamp 2 firmware code compatible with
ESPHome version 2021.10.0. As a result, configuration changes are needed to make the configuration work with
the new code.

There are basically two ways in which you can upgrade:

1. Start a new configuration
2. Upgrade your existing configuration

## 1. Start a new configuration

*This is definitely the simplest way*.

Copy the new `example.yaml` configuration file from the GitHub repository and start a new configuration based
on that. When this configuration is working, you can extend your configuration to tweak the configuration to
your needs.

## 2. Upgrade your existing configuration

If you have done extensive customization, it might be easier to upgrade your existing configuration. Below
you find a list of changse that have to be applied, and that should get your configuration ready for 2021.10.0.

**Setup the following substitutions in your configuration file:**

```yaml
substitutions:
  name: bedside-lamp
  friendly_name: Bedside Lamp
  light_name: ${friendly_name} RGBWW Light
  light_mode_text_sensor_name: ${friendly_name} Light Mode
  default_transition_length: 800ms
```

Other substitutions are not in use anymore, so you can delete them.

**Modify the existing `external_components:` configuration to:**

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/mmakaay/esphome-xiaomi_bslamp2
      ref: release/2021.10.0
    refresh: 60s
```

**Add the new core configuration package:**

```yaml
packages:
  bslamp2:
    url: https://github.com/mmakaay/esphome-xiaomi_bslamp2
    ref: release/2021.10.0
    files:
      - packages/core.yaml
    refresh: 0s
```
You can omit the ones that represent the required value already.

**Fully remove the existing `esphome:` configuration section**

Remove the following block from your configuration. The `core.yaml` configuration
package provides the required configuration, so you don't need it in your
configuration file anymore.

```yaml
esphome:
  name: ${name}
  ...
  etc.
```

**Modfy all instances of id's from a variable to a string value:**

- `${id_light}` -> `my_light`
- `${id_front_panel_illumination}` -> `my_front_panel_illumination`
- `${id_light_mode}` -> `my_light_mode`
- `${id_power_button}` -> `my_power_button`
- `${id_color_button}` -> `my_color_button`
- `${id_slider_level}` -> `my_slider_level`

These are the identifiers that I will be using from now on in the configuration
packages as well. By using these identifiers in all places, it will be easy to
distribute configuration packages that can be added to your configuration.

**Fully remove the front panel illumination output from the config**

This configuration is already being setup as part of the `core.yaml` configuration package.
When not removing it, you would get an error about the component being redefined. So simply
remove this block from your configuration:

```yaml
output:
  - platform: xiaomi_bslamp2
    id: my_front_panel_illumination
```

**Optionally, modify the the name of the light component**

For the name of the light, you can replace `${friendly_name} RGBWW Light` with
the new `${light_name}` substitution variable.

```yaml
light:
  - platform: xiaomi_bslamp2
    id: my_light
    name: ${light_name}
    ...
```

** Optionally, modify the name of the text sensor**

Likewise, you can change the text sensor that publishes the current light mode as text.
The name can be changed from `${fiendly_name} Light Mode` to `${light_mode_text_sensor_name}`.

```yaml
text_sensor:
  - platform: xiaomi_bslamp2
    id: my_light_mode
    name: ${light_mode_text_sensor_name}
```
