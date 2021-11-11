< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >

# Installation guide

The code must be compiled into a firmware using ESPHome 2021.10.0 or later. Therefore, a prerequisite
is that you have ESPHome up and running in some form (command line, docker container, web dashboard,
possibly from within Home Assistant as an add-on). For information on this, please refer to the
documentation on the [ESPHome website](https://esphome.io).

The component code is distributed directly from GitHub. You will not have to download and install
the code manually. This leverages the [external components](https://esphome.io/components/external_components.html)
feature.

Before you can compile the firmware, you will have to create the YAML configuration file for your
device. You can take the [`example.yaml`](../example.yaml) from this repository as a starting point, and
modify that one to your needs. Detailed information about the YAML configuration options can be
found in the [Configuration guide](configuration.md).

After these steps you can let ESPHome compile your firmware (`firmware.bin`) file. This firmware
can then be flashed onto the device.

**Note**: During compilation you might see a warning message from ESPHome, telling you:
```
WARNING GPIO4 is a Strapping PIN and should be avoided.
```
You can safely ignore this warning. GPIO4 is hard-wired in the lamp's PCB and the original firmware
uses it as the master switch for turning on and off the LEDs. This way of using the pin does not
pose any problems for the device.

Like normal with ESPHome, the first time you will have to flash the device using a serial interface.
After this initial flashing operation, you can flash new versions of the firmware using the OTA
(Over The Air) method.

See [the flashing guide](flashing.md) for hints on opening up the device and flashing its firmware
using the serial interface.

< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >
