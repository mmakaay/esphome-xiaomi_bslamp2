# esphome-yeelight_bs2

## Installation

Create a folder named `custom_components` in the folder where your
device's yaml configuration file is stored. Then clone the the Github
repo into a subfolder `yeelight_bs2`. For example on the command line:

```
# mkdir custom_components
# cd custom_components
# git clone https://github.com/mmakaay/esphome-yeelight_bs2 yeelight_bs2
```

Your folder structure should now look like:
```
config
├── yourdevice.yaml
├── custom_components/
│   ├── yeelight_bs2/
│   .   ├── README.md
.   .   ├── yeelight_bs2_light_output.h
.   .   .
```

Then add the required configuration to your device's yaml configuration file.
For an example file, take a look at `doc/example.yaml` in this repository.

On a Rapsbery Pi with HomeAssistant and ESPHome as a plugin, the directory should be:


/config/esphome/custom_components/yeelight_bs2/

```
config
├── epshome
│   ├── yourdevice.yaml
│   ├── custom_components/
|   .   ├── yeelight_bs2/
│   .   .    ├── README.md
.   .   .    ├── yeelight_bs2_light_output.h
.   .   .    .
```

See [doc/FLASHING.md](doc/FLASHING.md) for hints for opening and flashing the light.

## Issue: the device keeps losing its connection to Home Assistant

This is not a problem with the device or the custom firmware, but a problem
in the upstream library "AsyncTCP". I did identify the issue and have a
proposed fix for it. The issue was reported at:

https://github.com/me-no-dev/AsyncTCP/issues/116

If you want to try out this change, then create a `libs` folder in the
folder where your device's yaml configuration file is stored, and clone the
following repository into that folder:

  https://github.com/mmakaay/AsyncTCP

For example on the command line:

```
# mkdir libs
# cd libs
# git clone://github.com/mmakaay/AsyncTCP
```

Then add a pointer to this folder from within your device's yaml
configuration file, using the `lib_extra_dirs` option. Provide it with the
absolute path to your `libs` folder. The relevant part of the config change
looks like this:

```yaml
esphome:
  platformio_options:
    lib_extra_dirs: /config/libs
```

This way, the repository version of the library will override the version of
the library that is bundled with ESPHome. Build the device firmware and
flash the device like you would normally do.

