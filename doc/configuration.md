[Index](../README.md) | [Flashing guide](flashing.md) >

# Configuration guide

The `xiaomi_bslamp2` platform provides various components that expose the core functionalities of the lamp.
In the following table, you can find what components are used for exposing what parts of the lamp.

  | Part                       | Component(s)                                           |
  | -------------------------- |--------------------------------------------------------|
  | ESP32 pinouts              |  [platform 'xiaomi_bslamp2'](#platform-xiaomi-bslamp2) |
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
Therefore, you will not find this piece of configuration in the [example.yaml](doc/example.yaml).

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

## Component: binary_sensor

## Component: sensor

## Component: output

## Component: text_output
