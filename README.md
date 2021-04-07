# esphome-yeelight_bs2

## Warning: this code is still under heavy development

This code is not yet production-ready. Most of the work goes into reverse
engineering the original firmware and coming up with ways to re-implement
a device firmware based on ESPHome.

Till now, the main focus was driving the LED circuitry, to make sure that the
light quality meets that of the original firmware. This was a hard nut to
crack, but I am happy to announce that this code has been completed.
Therefore I will now continue on implementing the front panel buttons.

The hard work for this was already done: reverse engineering the protocol
that is used to talk to the main board. Therefore, finishing up the
firmware should not take much time from here on.

Once the front panel is working, we can move towards a stable release of the
firmware. I have some more ideas to work on, but those can and will be
extensions to a finalized first stable release.


## Is it safe to install this firmware on my device?

As long as you keep a backup of the original Yeelight firmware, this is
quite safe :-)

I have two lamps that both are running the latest development firmware and
they are functioning very well as far as the light feature is concerned.
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

