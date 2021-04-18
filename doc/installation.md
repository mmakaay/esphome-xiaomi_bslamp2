< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >

# Installation guide

The code must be compiled using ESPHome. Therefore, a prerequisite is that
you have ESPHome up and running in some form (command line, docker container,
web dashboard, possibly from within Home Assistant).
For information on this, please refer to the documentation on the
[ESPHome website](https://esphome.io).

Create a folder named `custom_components` in the folder where your device's
yaml configuration file is stored. Then clone the the Github repo into a
subfolder `xiaomi_bslamp2`. For example on the command line:

```
# cd /your/path/to/config
# mkdir custom_components
# cd custom_components
# git clone https://github.com/mmakaay/esphome-xiaomi_bslamp2 xiaomi_bslamp2
```

Your folder structure should now look like:
```
config
├── yourdevice.yaml
├── custom_components/
│   ├── xiaomi_bslamp2/
│   .   ├── README.md
.   .   ├── LICENSE.md
.   .   .
```

On a Rapsbery Pi with HomeAssistant and ESPHome as a plugin, the directory
should be: `/config/esphome/custom_components/xiaomi_bslamp2/`

```
config
├── esphome
│   ├── yourdevice.yaml
│   ├── custom_components/
|   .   ├── xiaomi_bslamp2/
│   .   .    ├── README.md
.   .   .    ├── LICENSE.md
.   .   .    .
```

Then create the required configuration in your device's yaml configuration
file. For an example file, take a look at the example file
[doc/example.yaml](doc/example.yaml) in this repository.
Detailed configuration instructions can be found in the
[Configuration guide](doc/configuration.md).

After these steps you can compile your firmware `firmware.bin` file.
This firmware can then be flashed onto the device.

Like normal with ESPHome, the first time you will have to flash the
device using a serial interface. After this initial flashing operation, you
can flash new versions of the firmware using the OTA (Over The Air) method.

See [doc/flashing.md](the flashing guide) for hints on opening up the
device and flashing its firmware using its serial interface.

< [Why custom firmware?](why_custom_firmware.md) | [Index](../README.md) | [Configuration guide](configuration.md) >
