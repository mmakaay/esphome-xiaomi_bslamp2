< [Installation guide](installation.md) | [Index](../README.md) | [Flashing guide](flashing.md) >

# Configuration guide

I think, the best starting point for creating your own yaml configuration, is to look at the
[`example.yaml`](../example.yaml) file and the [configuration packages](../packages).
These configuration files were written with the functionality of the original firmware in
mind and it makes use of all available options. This configuration guide can be used to fill
in the blanks.

The `xiaomi_bslamp2` platform provides various components that expose the core functionalities of
the lamp. In the following table, you can find what components are used for exposing what physical
components of the lamp.

  | Part                       | Component(s)                                                     |
  | -------------------------- |------------------------------------------------------------------|
  | ESP32 pinouts              |  [platform xiaomi_bslamp2](#platform-xiaomi_bslamp2)             |
  | RGBWW LEDs                 |  [light](#light)                                                 |
  | Front Panel Power button   |  [binary_sensor](#component-binary_sensor)                       |
  | Front Panel Color button   |  [binary_sensor](#component-binary_sensor)                       |
  | Front Panel Slider         |  [binary_sensor](#component-binary_sensor) (touch/release)       |
  |                            |  [sensor](#component-sensor) (touched slider level)              |
  | Front Panel Illumination   |  [output](#component-output) (on/off + indicator level)          |
  | Light mode propagation     |  [text_sensor](#component-text_sensor)                           |

## Platform: xiaomi_bslamp2

At the core of the hardware support is the `xiaomi_bslamp2` platform, which provides two hub-style
hardware abstraction layer (HAL) components that are used by the other components: one for driving
the GPIO's for the RGBWW leds and one for the I2C communication between the ESP32 and the front
panel.

You will not have to add any configuration options for the `xiaomi_bslamp2` to your yaml file.
The required configuration is fully provided by the configuration package `packages/core.yaml`.
The GPIO + I2C configurations are prepared to work for the Bedside Lamp 2 wiring out of the box.

```yaml
xiaomi_bslamp2:
```

The only reason that I can think of for adding this platform configuration to your yaml file, would
be if you blew one or more or the ESP32 pins, and need to rewire functionality. In other cases,
simply omit the section.

## Component: light

The light component creates an RGBWW light. This means that it can do colored light and cold/warm
white light based on a color temperature.

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
  you can reference the light from automation rules (e.g. to turn on the light when the power button
  is tapped)
* **default_transition_length** (*Optional*, Time): The transition length to use when no transition
  length is set in a light call. Defaults to 1s.
* **effects** (*Optional*, list): A list of
  [light effects](https://esphome.io/components/light/index.html#light-effects) to use for this light.
* **presets** (*Optional*, dict): Used to define presets, that can be used from automations. See
  [below](#light-presets) for detailed information.
* **on_brightness** (*Optional*, Action): An automation to perform when the brightness of the light
  is modified.
* All other options from
  [the base Light implementation](https://esphome.io/components/light/index.html#config-light),
  except for options that handle color correction options like `gamma_correct` and `color_correct`.
  These options are superceded by the fact that the light component has a fully customized light
  model, that closely follows the light model of the original lamp's firmware.

### Light modes

The lamp supports multiple light modes. These are:

* **RGB light** (input: RGB + brightness > 1%)
* **White light** (input: Color Temperature + brightness > 1%)
* **Night light** (input: RGB or White light + brightness at 1%)

In the original firmware + Yeelight Home Assistant integration, the night light feature is
implemented through a switch component. The switch can be turned on to activate the night light
mode. In this ESPHome firmware, setting the brightness to its lowest value triggers the night light
mode. This makes things a lot easier to control.

It is possible to control the night light mode separately. An example of this can be found in the
[example.yaml](../example.yaml), in which holding the power button is bound to activating the night
light.

### `light.disco_on` Action

This action sets the state of the light immediately (i.e. without waiting for the next main loop
iteration), without saving the state to memory and without publishing the state change.

```yaml
on_...:
  then:
    - light.disco_on:
        id: my_bedside_lamp
        brightness: 80%
        red: 70%
        green: 0%
        blue: 100%
```

The possible configuration options for this Action are the same as those for the standard
`light.turn_on` Action.

### `light.disco_off` Action

This action turns off the disco mode by restoring the state of the lamp to the last known state from
before using the disco mode.

```yaml
on_...:
  then:
    - light.disco_off:
        id: my_bedside_lamp
```

### Light presets

The presets functionality was written with the original lamp firemware functionality in mind: the
user has two groups of presets available: one for RGB light presets and one for white light presets
(based on color temperature). The color button (the top one on the front panel) can be tapped to
switch to the next preset within the active preset group. The same button can be held for a little
while, to switch to the other preset group.

In your light configuration, you can mimic this behavior (in fact: it is done so in the
[example.yaml](../example.yaml)) by means of the presets system. This system consists of two parts:

* Defining presets
* Activating presets from automations

**Defining presets**

Presets can be configured in the `presets` option of the `light` configuration.

Presets are arranged in groups. You can define as little or as many groups as you like. The example
configuration uses two groups, but that is only to mimic the original behavior. If you only need one
group, then create one group. If you need ten, go ahead and knock yourself out.

The general structure of the presets configuration is:

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

*Note: Duplicate template names are ok, as long as they are within their own group. If you use
duplicate preset names within a single group, then the last preset will override the earlier
one(s).*

A preset can define one of the following:

* **RGB light**
  * **red** (**Optional**, percentage): the red component of the RGB value (default = 0%).
  * **green** (**Optional**, percentage): the green component of the RGB value (default = 0%).
  * **blue** (**Optional**, percentage): the blue component of the RGB value (default = 0%).
  * **brightness** (*Optional*, percentage): the brightness to use (default = current brightness).
  * **transition_length** (*Optional*, time): the transition length to use.
* **White light**
  * **color_temperature** (**Required**, mireds): the color temperature in mireds (range: "153 mireds" - "588 mireds")
  * **brightness** (*Optional*, percentage): the brightness to use (default = current brightness).
  * **transition_length** (*Optional*, time): the transition length to use.
* **Light effect**
  * **effect** (**Required**, string): the name of a light effect to activate.
* **Brightness change only**
  * **brightness** (**Required**, percentage): the brightness to use.

**Activating presets from automations**

Once presets have been configured, they can be activated using the `preset.activate` action. The
following options are available for this action:

* Switch to next preset group (and after the last, switch to the first):
```yaml
preset.activate:
  next: group
```

* Switch to next preset within currentl preset group (and after the last,
  switch to the first):
```yaml
preset.activate:
  next: preset
---

* Activate a specific preset group by specifying the group's name:
```yaml
preset.activate:
  group: rgb
```

* Activate a specific preset by specifying both the preset's name and group name:
```yaml
preset.activate:
  group: white
  preset: warm
```

Shorthand definitions are available for all these actions:

```yaml
preset.activate: next_group
preset.activate: next_preset
preset.activate: rgb
preset.activate: white.warm
```

**Handling of invalid input**

When a group or template is specified that does not exist, or if next group/preset is used while no
presets have been defined at all, then the action will be ignored and an error will be logged.

## Component: binary_sensor

Binary sensors can be added to the configuration for handling touch/release events for the front
panel. On touch, a binary_sensor will publish `True`, on release it will publish `False`. The
configuration of a binary_sensor determines what part of the front panel is involved in the touch
events.

```yaml
binary_sensor:
  - platform: xiaomi_bslamp2
    id: my_bedside_lamp_power_button
    for: POWER_BUTTON
    on_press:
      then:
        - light.toggle: my_bedside_lamp
```

For referencing the parts of the front panel, the following part identifiers are available:

* POWER_BUTTON (or its alias: POWER)
* COLOR_BUTTON (or its alias: COLOR)
* SLIDER

If personal taste dictates so, you can use lower case characters and spaces instead of underscores.
This means that for example "Power Button" and "power" would be valid identifiers for the power
button.

### Configuration variables:

* **name** (*Optional*, string): The name of the binary sensor. Setting a name will expose the
  binary sensor as an entity in Home Assistant. If you do not need this, you can omit the name.
* **id** (*Optional*, ID): Manually specify the ID used for code generation. By providing an id,
  you can reference the binary_sensor from automation rules (to retrieve the current state of the
  binary_sensor).
* **for** (*Mandatory*, part identifier): This specifies to for part of the front panel the binary
  sensor must report touch events.
* All other options from
  [Binary Sensor](https://esphome.io/components/binary_sensor/index.html#config-binary-sensor).

## Component: sensor

The sensor component publishes touch events for the front panel slider. The published value
represents the level at which the slider was touched.

*Note: This sensor only reports the touched slider level. It cannot be used for detecting release
events. If you want to handle touch/release events for the slider, then you can make use of the
[binary_sensor](#component-binary_sensor) instead.*

```yaml
sensor:
  - platform: xiaomi_bslamp2
  - id: my_bedside_lamp_slider_level
    range_from: 0.2
    range_to: 0.9
    on_value:
      then:
        - light.turn_on:
            id: my_bedside_lamp
            brightness: !lambda return x;    
```

### Configuration variables:

* **name** (*Optional*, string): The name of the sensor. Setting a name will expose the sensor as an
  entity in Home Assistant. If you do not need this, you can omit the name.
* **id** (*Optional*, ID): Manually specify the ID used for code generation. By providing an id,
  you can reference the sensor from automation rules (e.g. to retrieve the current state of the
  binary_sensor).
* **range_from** (*Optional*, float): By default, published values vary from the range 0.01 to 1.00,
  in 20 steps. This option modifies the lower bound of the range.
* **range_to** (*Optional*, float): This option modifies the upper bound of the range.
* All other options from
  [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).

## Component: output

The (float) output component is linked to the front panel illumination + level indicator. Setting
this output (using the standard `output.set_level` action) to value 0.0 will turn off the frontpanel
illumination. Other values, up to 1.0, will turn on the illumination and will set the level indicator
to the requested level (in 10 steps).

```yaml
output:
  - platform: xiaomi_bslamp2
    id: my_bedside_lamp_front_panel_illumination
```

### Configuration variables:

* **id** (**Required**, ID): The id to use for this output component.
* All other options from [Output](https://esphome.io/components/output/index.html)

### Addressing the LEDs of the illumination individually

While the standard `output.set_level` action emulates the front panel illumination behavior
of the original device firmware, it is also possible to control all of the LEDs for this
illumination individually, in case you need some different behavior, e.g. leaving the
power button on at night, so the user can easily find it in the dark.

To address the LEDs, the following identifiers can be used in your YAML configuration:

* `POWER` : The power button illumination.
* `COLOR` : The color button illumination.
* `1`, `2`, .., `10` : The 10 LEDs on the slider, where LED `1` is closest to the
  power button and LED `10` is closest to the color button.
* `ALL` : represents all of the available LEDs
* `NONE` : represents none of the available LEDs

#### `front_panel.set_leds` Action

This action turns on the provided LEDs, all other LEDs are turned off.

```yaml
    on_...:
      then:
        - front_panel.set_leds:
            leds:
              - POWER
              - COLOR
              - 1
              - 2
              - 3
```

The `leds:` key can also be omitted here, making the following action calls
equivalent to the one above.

```yaml
    on_...:
      then:
        - front_panel.set_leds:
            - POWER
            - COLOR
            - 1
            - 2
            - 3
```
 
This can also be written as:

```yaml
    on_...:
      then:
        - front_panel.set_leds: [ POWER, COLOR, 1, 2, 3 ]
```

If only one LED is specified, you are allowed to omit the list definition:

```yaml
    on_...:
      then:
        - front_panel.set_leds: POWER
```

#### `front_panel.turn_on_leds` Action

This action turns on the provided LEDs, and leaves the rest of the LEDs as-is.
The LEDs to affect are specified in the same wat as above for `front_panel.set_leds`.

#### `front_panel.turn_off_leds` Action

This action turns off the provided LEDs, and leaves the rest of the LEDs as-is.
The LEDs to affect are specified in the same wat as above for `front_panel.set_leds`.

#### `front_panel.set_level` Action

This action works like the `output.set_level` action, but it only updates the
LEDs of the slider. The LEDs for the power and color button are left as-is.

```yaml
    on_...:
      then:
        - front_panel.set_level: 0.5
```

#### `front_panel.update_leds` Action

The previous actions only modify the required state for the front panel LEDs.
Updating the actual state of the LEDs is done when the main loop for the
output component is run by ESPHome.

If you need the required state to be pushed to the LEDs immediately, regardless
the main loop, then this action can ben used to take care of this.

*Note: In most situations, you will not need to use this action explicitly
to make the LEDs update. Only use it when you are sure that this is required.*

```yaml
    on_...:
      then:
        - front_panel.set_leds: POWER
        - front_panel.update_leds:
```

## Component: text_sensor

The text sensor component publishes changes in the active [light mode](#light-modes). Possible
output values for this sensor are: "off", "rgb", "white" and "night".

### Configuration variables:

* **name** (*Optional*, string): The name of the text sensor. Setting a name will expose the text
  sensor as an entity in Home Assistant. If you do not need this, you can omit the name.
* **id** (*Optional*, ID): Manually specify the ID used for code generation. By providing an id,
  you can reference the text sensor from automation rules (to retrieve the current state of the
  text_sensor).
* All other options from
  [Text Sensor](https://esphome.io/components/text_sensor/index.html)

< [Installation guide](installation.md) | [Index](../README.md) | [Flashing guide](flashing.md) >
