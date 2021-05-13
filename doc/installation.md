< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >

# Installation guide

The code must be compiled into a firmware using ESPHome. Therefore, a prerequisite is that you have
ESPHome up and running in some form (command line, docker container, web dashboard, possibly from
within Home Assistant as an add-on). For information on this, please refer to the documentation on
the [ESPHome website](https://esphome.io).

The component code is distributed directly from GitHub. You will not have to download and install
the code manually. This leverages the external components feature that was introduced in ESPHome
version 1.18.0. Therefore, you must use ESPHome version 1.18.0 or later.

Before you can compile the firmware, you will have to create the YAML configuration file for your
device. You can take the [example.yaml](example.yaml) from this repository as a starting point, and
modify that one to your needs. Detailed information about the YAML configuration options can be
found in the [Configuration guide](configuration.md).

After these steps you can let ESPHome compile your firmware (`firmware.bin`) file. This firmware
can then be flashed onto the device.

Like normal with ESPHome, the first time you will have to flash the device using a serial interface.
After this initial flashing operation, you can flash new versions of the firmware using the OTA
(Over The Air) method.

See [the flashing guide](flashing.md) for hints on opening up the device and flashing its firmware
using the serial interface.

< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >
