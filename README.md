# esphome-xiaomi_bslamp2

## Warning: this code is still under development

This code might not yet be production-ready.
At this point, it is declared beta-quality.

The new firmware is currently being tested and depending on the actual
use cases and issues that we might run into, breaking changes could still
occur at some point.


## Is it safe to already install this firmware on my device?

As long as you keep a backup of the original firmware, this is quite safe :-)
I have two lamps that both are running the latest development firmware and
they are functioning very well.

I sometimes see API disconnection issues, but those can all be traced back
to the underlying frameworks. For the most prevalent issue, I did some
debugging and wrote a fix (it is mentioned below).

For each commit of the code, I will do my best to commit it in a working
state. Once a first completed stable release is cooked up, I will tag
production releases of the code to make it easier to pick the safe version
for production purposes.


## Installation

The code must be compiled using ESPHome. Therefore, a prerequisite is that
you have ESPHome up and running in some form (command line, docker container,
web dashboard, possibly from within Home Assistant).
For information on this, please refer to the documentation on the
[ESPHome website](https://esphome.io).

Create a folder named `custom_components` in the folder where your device's
yaml configuration file is stored. Then clone the the Github repo into a
subfolder `xiaomi_bslamp2`. For example on the command line:

```
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

After these steps you can compile your firmware `firmware.bin` file.
This firmware can then be flashed onto the device.

Like normal with ESPHome, the first time you will have to flash the
device using a serial interface. After this initial flashing operation, you
can flash new versions of the firmware using the OTA (Over The Air) method.

See [doc/FLASHING.md](doc/FLASHING.md) for hints on opening up the device and
flashing its firmware using the serial interface.


## Issue: the device keeps losing its connection to Home Assistant

This is not a problem with the device or the custom firmware, but a problem
in the upstream library "AsyncTCP". I did identify an issue and my pull
request for a fix was accepted and merged:

  https://github.com/OttoWinter/AsyncTCP/pull/4

This fix will likely be available in the next release of ESPHome
(the current version at the time of writing is 1.16.2).
If you want to try out this fix on beforehand, then create a `libs` folder
in the folder where your device's yaml configuration file is stored (e.g.
when using the Home Assistant plugin: `/config/esphome/libs/`).
Then clone the following repository into that folder:

  https://github.com/OttoWinter/AsyncTCP

For example on the command line:

```
# cd /config/esphome
# mkdir libs
# cd libs
# git clone https://github.com/OttoWinter/AsyncTCP
```

Then add a pointer to this folder from within your device's yaml
configuration file, using the `lib_extra_dirs` option. Provide it with the
absolute path to your `libs` folder. The relevant part of the config change
looks like this (matching the example from above):

```yaml
esphome:
  platformio_options:
    lib_extra_dirs: /config/esphome/libs
```

This way, the repository version of the library will override the version of
the library that is bundled with ESPHome. Build the device firmware and
flash the device like you would normally do.
