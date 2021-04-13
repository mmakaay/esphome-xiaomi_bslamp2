# esphome-yeelight_bs2

## Warning: this code is still under development

This code might not yet be production-ready.

At this point, I would call it alpha-quality. Mainly in the sense that the
interfaces that have been implemented so far might still change.
The new firmware is currently being tested and depending on the actual
use cases and issues that we might run into, breaking changes could still
be done at some point.

## Is it safe to install this firmware on my device?

As long as you keep a backup of the original Yeelight firmware, this is
quite safe :-)

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

Create a folder named `custom_components` in the folder where your device's
yaml configuration file is stored. Then clone the the Github repo into a
subfolder `yeelight_bs2`. For example on the command line:

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
For an example file, take a look at [doc/example.yaml](doc/example.yaml) in
this repository.

On a Rapsbery Pi with HomeAssistant and ESPHome as a plugin, the directory
should be:


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

See [doc/FLASHING.md](doc/FLASHING.md) for hints for opening up the device and
flashing its firmware.


## Issue: the device keeps losing its connection to Home Assistant

This is not a problem with the device or the custom firmware, but a problem
in the upstream library "AsyncTCP". I did identify an issue and my pull
request for a fix was accepted and merged:

   https://github.com/OttoWinter/AsyncTCP/pull/4

If you want to try out this fix, straight from my repository, lhen create a 
`libs` folder in the folder where your device's yaml configuration file is
stored, and clone the following repository into that folder:

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

