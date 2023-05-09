# ESPHome support for Xiaomi Mijia Bedside Lamp 2

[ [Changelog](CHANGELOG.md) | [License](LICENSE.md) | [Code of conduct](CODE_OF_CONDUCT.md) ]

The Bedside Lamp 2 is a smart RGBWW LED lamp, produced by Yeelight for the Xiaomi Mijia brand. It
can be controlled via the WiFi network and from a touch panel on the front of the device. The touch
panel contains a power button, a button that changes the color of the light and a slider that can be
used to change the brightness of the light.

This project provides custom components for ESPHome, which make it possible to fully control every
aspect of the lamp and to integrate the lamp in your Home Assistant setup.

## Features

* The lamp **integrates easily with Home Assistant** using the ESPHome integration.

* **The lamp no longer phones home to the Mijia Cloud.** Using this firmware, you can rest assured
  that the network traffic is limited to your own network.  This matches the ideas behind Home
  Assistant, of providing a local home automation platform, that puts privacy first.

* **No more need for the LAN control option** to integrate the lamp with Home Assistant. Especially
  important, because Xiaomi decided in all their wisdom to remove LAN control from the device,
  breaking existing integrations.

* **The night light supports multiple colors**. The original firmware only supports a single warm
  white night light color.

* **Smooth light color transitions**, unlike the current version of the Yeelight integration. The
  Homekit integration does provide good transitions, but on my system, the color temperature white
  light mode is missing in the Home Assistant GUI.

* **Since the components of the lamp are exposed as ESPHome components, you don't have to stick with
  the lamp's original behavior.** You can hook up the lamp in your home automation as you see fit.
  Use the slider to control the volume of your audio set? Long press the power button to put your
  house in night mode? Use the illumination behind the slider bar to represent the progress of your
  sour dough bread bulk fermentation?  Go ahead, make it so! :-)

* **All LEDs that are used for illumination of the front panel (power button, color button and
  10 LEDs for the brightness slider) can be controlled individually.** This means that you have
  12 LEDs in total to use as you see fit, instead of sticking with the behavior of the original
  firmware.

* **Possibilities to extend the device's functionality through hardware mods.** There are [GPIO pins
  that are not in use](doc/technical_details.md#esp32-pinout).  If "tinkerer" is your middle name,
  you can use those pins to come up with your own hardware hacks to extend the device's
  functionality. 

## Quick start guide

For those who have experience with flashing ESPHome onto devices:

* Make sure you are using ESPHome 2023.4.0 or newer.
* Copy [`example.yaml`](example.yaml) to `<CONFIG_DIR>/<NODE_NAME>.yaml`.
* Modify the configuration to your needs (see the [configuration guide](doc/configuration.md)).
* Compile the `firmware.bin` file and download it to the device to which you have connected your
  serial to USB adapter.
* [Open up the lamp](doc/flashing.md#opening-the-lamp-to-expose-the-pcb) and connect its `TX`, `RX`,
  `GND` and `GPIO0` debug pads to the serial adapter. Check this [image for the debug pad
  locations](doc/images/09_debug_pads_for_soldering.jpg).
* Power up the lamp with `GPIO0` connected to GND to enable flashing mode.
* Flash `firmware.bin` onto the device, for example using
  [esphome-flasher](https://github.com/esphome/esphome-flasher)..
  
## Upgrading

I always try to make it possible to upgrade the lamp firmware without having to do anything special.
Sometimes, this is not possible. For those cases, the upgrade instruction can be found here:

* [Upgrading to 2021.10.0](doc/upgrading_to_2021.10.0.md)

## Table of contents

* [Why custom firmware?](doc/why_custom_firmware.md)
* [Installation guide](doc/installation.md)
* [Configuration guide](doc/configuration.md)
* [Flashing guide](doc/flashing.md)
* [Technical details](doc/technical_details.md)
* [Sponsoring](doc/sponsoring.md)
