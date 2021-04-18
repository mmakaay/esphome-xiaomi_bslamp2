# ESPHome component for Xiaomi Mijia Bedside Lamp 2

The Bedside Lamp 2 is a smart RGBWW LED lamp, produced by Yeelight. It can
be controlled via the WiFi network and using a touch panel on the front of
the device. The touch panel contains a power button, a button that changes
the color of the light and a slider that can be used to change the
brightness of the light.

This project provides a custom component for ESPHome, which makes it
possible to fully control every aspect of the lamp.

## Quick start guide

For those who have experience with flashing ESPHome onto devices, here's a
quick start guide:

* Clone the [GitHub repo](https://github.com/mmakaay/esphome-xiaomi_bslamp2)
  into your ESPHome `config/custom_components` directory.
* Copy `config/custom_components/xiaomi_bslamp2/doc/example.yaml` to
  `config/your_device_name.yaml` and modify it to your needs.
* Compile the `firmware.bin` file and download it to the device to which you
  have connected a serial to USB adapter (FTDI).
* Open up the lamp and connect its `TX`, `RX`, `GND` and `GPIO0` debug pads
  to the serial adapter (see the [detailed installation
  guide](doc/install.md) for the debug pad locations).
* Power up the lamp with `GPIO0` connected to GND to enable flashing mode.
* Flash `firmware.bin` onto the device.

If you experience regular disconnects between Home Assistant and the lamp,
then take a look at the [known issues document](doc/known_issues.md).

## Table of contents:

* [Why custom firmware?](doc/why_custom_firmware.md)
* [Installation guide](doc/installation.md)
* [Configuration_guide](doc/configuration.md)
* [Flashing guide](doc/flashing.md)
* [Known issues](doc/known_issues.md)

