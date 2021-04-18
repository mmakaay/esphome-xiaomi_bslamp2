< [Flashing guide](flashing.md) | [Index](../README.md) | [Sponsoring](sponsoring.md) >

# Known issues

## The device keeps losing its connection to Home Assistant

Disconnects are annoying, but even more annoying is that sometimes the
lamp will reboot (during which the light will be turned off) as a result of
these disconnects. The reasons for these reboots are very likely in the
underlying libraries, and I'm working on tracking these down.

In the meanwhile, there are a few factors that you can look at to bring down
the number of disconnects:

* A bug in the AsyncTCP library
* The number of connected API clients
* The logging output level

**A bug in the AsyncTCP library**

You might be running into a problem in the upstream library "AsyncTCP".
If you connect a serial console to your lamp and see "ack timeout 4"
messages in the logging output before the lamp disconnects the API client,
then you can be pretty sure that this is the culprit.

I did identify the underlying issue and a pull request for a fix was
accepted and merged into the ESPHome fork of the library:

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

**The number of connected API clients**

Another factor on the connection stability, seems to be the number of
clients that are connected to the API. When connecting a log client via the
API (e.g. when looking at the logging from within the web dashboard), while
Home Assistant is also connected, disconnects might occur.

When the lamp is in production, this is not an issue. Then only Home
Assistant is connected.

**The logging output level**

I have seen an increase in disconnects while the log level was set to
`VERY_VERBOSE`. The logging code might take up so much time, that it
interferes with the operation of the networking code. Especially since the
ESP32-WROOM-32D chip that is used in the lamp is a single core version of
the ESP32.

For this reason, I advise to completely omit logging or use a very low log
level for production purposes.

< [Flashing guide](flashing.md) | [Index](../README.md) | [Sponsoring](sponsoring.md) >
