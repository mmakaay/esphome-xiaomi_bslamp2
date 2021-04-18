< [Installation guide](installation.md) | [Index](../README.md) | [Flashing guide](flashing.md) >

# Configuration guide

I think, the best starting point for creating your own yaml configuration, is to
look at the [example.yaml](example.yaml) file from the project documentation.
This configuration was written with the functionality of the original firmware in mind
and it makes use of all available options. This configuration guide can be used to
fill in the blanks.

The `xiaomi_bslamp2` platform provides various components that expose the core functionalities of the lamp.
In the following table, you can find what components are used for exposing what parts of the lamp.

  | Part                       | Component(s)                                           |
  | -------------------------- |--------------------------------------------------------|
  | ESP32 pinouts              |  [platform xiaomi_bslamp2](#platform-xiaomi_bslamp2)   |
  | RGBWW LEDs                 |  [light](#light)                                       |
  | Front Panel Power button   |  [binary_sensor](#binary_sensor)                       |
  | Front Panel Color button   |  [binary_sensor](#binary_sensor)                       |
  | Front Panel Slider         |  [binary_sensor](#binary_sensor) (touch/release)       |
  |                            |  [sensor](#sensor) (touched slider level)              |
  | Front Panel Illumination   |  [output](#output) (on/off + indicator level)          |
  | Light mode propagation     |  [text_sensor](#text_sensor)                           |

## Platform: xiaomi_bslamp2

At the core of the hardware support is the `xiaomi_bslamp2` platform, which provides two
hub-style hardware abstraction layer (HAL) components that are used by the other components:
one for driving the GPIO's for the RGBWW leds and one for the I2C communication between
the ESP32 and the front panel.

I do mention it here for completeness sake, but generally you will not have to add the
following configuration option to your yaml file. It is loaded automatically by the
components that need it, and the GPIO + I2C configurations are fully prepared to work
for the Bedside Lamp 2 wiring out of the box.
Therefore, you will not find this piece of configuration in the [example.yaml](example.yaml).

Having said that, here are the configuration options:

```yaml
xiaomi_bslamp2:
  # Options for the RGBWW LEDs HAL.
  red: "GPIO13"
  green: "GPIO14"
  blue: "GPIO5"
  white: "GPIO12"
  master_1: "GPIO33"
  master_2: "GPIO4"
  
  # Options for the Front Panel HAL.
  sda: "GPIO21"
  scl: "GPIO19"
  address: 0x2C
  trigger_pin: "GPIO16"
```

The only reason that I can think of for adding this platform configuration to your yaml
file, would be if you blew one or more or the ESP32 pins, and need to rewire functions
to different pins.

## Component: light

The light component creates an RGBWW light. This means that it can do colored light and
cold/warm white light based on a color temperature.

```yaml
light:
  - platform: xiaomi_bslamp2
    name: My Bedside Lamp
    id: my_bedside_lamp
    default_transition_length: 0.5s
    effects:
      - random:
          name: Randomize
          transition_length: 3s
          update_interval: 3s
    on_brightness:
      - then:
          - logger.log: The brightness changed!
    presets:
      my_color_presets:
        red:         { red: 100%, green: 0%,   blue: 0%   }
        green:       { red: 0%,   green: 100%, blue: 0%   }
        blue:        { red: 0%,   green: 0%,   blue: 100% }
        yellow:      { red: 100%, green: 100%, blue: 0%   }
        purple:      { red: 100%, green: 0%,   blue: 100% }
        randomize:   { effect: Randomize                  }
      my_white_presets:
        cold:        { color_temperature: 153 mireds      }
        chilly:      { color_temperature: 275 mireds      }
        luke:        { color_temperature: 400 mireds      }
        warm:        { color_temperature: 588 mireds            
```

### Configuration variables:

* **name** (**Required**, string): The name of the light.
* **id** (*Optional*, ID): Manually specify the ID used for code generation. By providing an id,
  you can reference the light from automation rules (e.g. to turn on the light when the power
  button is tapped)
* **default_transition_length** (*Optional*, Time): The default transition length to use when
  no transition length is set in a light call. Defaults to 1s.
* **effects** (*Optional*, list): A list of [light effects](https://esphome.io/components/light/index.html#light-effects)
  to use for this light.
* **presets** (*Optional*, dict): Used to define presets, that can be used from automations.
  See [below](#light-presets) for detailed information.
* **on_brightness** (*Optional*, Action): An automation to perform when the brightness of the light is modified.
* All other options from [the base Light implementation](https://esphome.io/components/light/index.html#config-light),
  except for options that handle color correction options like `gamma_correct` and `color_correct`.
  These options are superceded by the fact that the light component has a fully customized
  light model, that closely follows the light model of the original lamp's firmware.

### Light presets

The presets functionality was written with the original lamp firemware functionality in mind:
the user has two groups of presets available: one for RGB light presets and one for white light
presets (based on color temperature). The color button (the top one on the front panel) can be
tapped to switch to the next preset within the active preset group. The same button can be
held for a little while, to switch to the other preset group.

In your light configuration, you can mimic this behavior (in fact: it is done so in the
[example.yaml](example.yaml)) by means of the presets system. This system consists of two
parts:

* Defining presets
* Activating presets from automations

**Defining presets**

Presets can be configured in the `presets` option of the `light` configuration.
Presets are arranged in groups. You can define as many groups as you like.
The example configuration uses two groups, but that is only to mimic the original behavior.
I you only need one group, then create one group. If you need ten, go ahead and knock yourself out.

The general structure of presets is:

```yaml
light:
  presets:
    group_1:
      preset_1: ...
      preset_2: ...
      ..
    group_2:
      preset_1: ...
      preset_2: ...
      ..
    ..
```

*Note: it is allowed to use duplicate template names, as long as the templates are in their own group.
If you use duplicate preset names within a single group, then the last preset will override the
earlier one(s).*

TODO



## Component: binary_sensor

## Component: sensor

## Component: output

## Component: text_output


< [Installation guide](installation.md) | [Index](../README.md) | [Flashing guide](flashing.md) >
