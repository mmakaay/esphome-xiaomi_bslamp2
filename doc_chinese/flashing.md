< [Configuration guide](configuration.md) | [Index](../README.md) | [Technical details](technical_details.md) >

# Flashing guide

Table of contents:
* [Warning](#warning)
* [Tools needed](#tools-needed)
* [Opening the lamp, to expose the PCB](#opening-the-lamp-to-expose-the-pcb)
* [Solder wires to the board](#solder-wires-to-the-board)
* [Connect the wires to your serial to USB adapter](#connect-the-wires-to-your-serial-to-usb-adapter)
* [When you only have one GND pin on your USB Adapter](#when-you-only-have-one-gnd-pin-on-your-usb-adapter)
* [Make a backup of the current firmware](#make-a-backup-of-the-current-firmware)
* [How to restore the backed up firmware](#how-to-restore-the-backed-up-firmware)
* [Flash new ESPHome firmware](#flash-new-esphome-firmware)
* [Troubleshooting flash](#troubleshooting-flash)

## Warning

We have writen these instructions with care, but we will give absolutely no warranty. Perhaps you
will destroy your lamp and your computer.


## Tools needed

* Allen key (2mm, 5/64") or torx (T8) screw driver
* Soldering Iron
* A serial to USB adapter (for example FTDI) that can provide 3.3V RX/TX signals
* Some wires
* Optional: sticky tape, hot glue gun, magnifying glass


## Opening the lamp, to expose the PCB

*Tip: you can click on all images from below to view them in full size.*

Remove the rubber pads from the bottom of the lamp, to get access to 4 screws that attach the bottom
to the rest of the lamp.

<img src="../images/01_unboxed.jpg" width="200"><img src="../images/02_remove_rubber_pads.jpg" width="200">

Note that you don't have to remove these pads fully. Once you can access the screws, you've gone far
enough.

<img src="../images/03_bolts_overview.jpg" width="200">

Unbolt the 4 screws which were hidden under the rubber pads.

<img src="../images/04_remove_bolts.jpg" width="200"><img src="../images/05_bolts.jpg" width="200">

Detach the bottom from the rest of the lamp, exposing the PCB. This might take a bit of force. Just
pull it off bit by bit, until it pops loose.

<img src="../images/06_pull_off_the_bottom.jpg" width="200"><img src="../images/07_bottom_removed.jpg" width="200">

<img src="../images/08_board_exposed.jpg" width="400">


## Solder wires to the board

The wires will be connected to the debug pads that are shown in the following image.

<img src="../images/09_debug_pads_for_soldering.jpg" width="400">

Many of the serial to USB adapter have some header pins to which you can connect the wires of a
device (no soldering required). Therefore, it might be useful to use dupont wire. Cut off one end,
strip the wire, tin the wire and solder it to the board. 

*Note: Whether to use male or female dupont wires depends on how you want to connect the serial
adapter. In this example, I have used male wires, so I could plug them into a breadbord.*

<img src="../images/10_stripped_dupont_wires.jpg" width="200">

Solder the wires to the `RX`, `TX`, `GND` and `GPIO0` debug pads.
Beware not to use too much solder on the GPIO0 pad, because that might flow onto the pad that is right
next to it, permanently putting the device in flash mode as a result.

*Note: The board has a debug pad that exposes 3.3V. Do not use this pad to power the board from your
serial adapter. Always power the lamp using its own power supply.*

A few tips:

- Depending on the quality of your eyes, you might want to use a magnifying glass for the soldering
  work. Use one that is mounted on a stand, or you will quickly be left wishing that you could grow
  a third arm.
- You could use some sticky tape to fixate the cables before soldering.
- When you want to keep the wires attached after flashing the new firmware (e.g. for serial logging
  or for future firmware flashing), then you might want to apply some hot glue to fixate the wires.
  This prevents the wires from breaking off, due to excessive movement.

<img src="../images/11_soldered_wires.jpg" width="200"><img src="../images/12_optional_hot_glue.jpg" width="200">


## Connect the wires to your serial to USB adapter

Make sure that your adapter uses 3.3V for the RX/TX pins that you will connect to the lamp. Some of
these adapters allow you to switch between 3.3V and 5V using a switch or a jumper. Do not use an
adapter that only provides 5V output. Reason for this, is that the ESP32 chip works at 3.3V. I have
seen the chips accept 5V serial input (I did flash the lamp successfully like that), but I am not
sure if they are actually 5V tolerant. Better safe than sorry in such case!

The wires must be connected as follows:

  | Soldering point| Serial USB Adapter name  |
  | -------------- |:------------------------:|
  | GND            |  GND                     |
  | TX             |  RX                      |
  | RX             |  TX (3.3V)               |
  | GPIO0          |  GND                     |

To be able to flash the lamp, `GPIO0` must be connected to ground while the lamp boots up.
Therefore, connect these wires *before* plugging in the lamp's power supply. Flashing will *not*
work if you connect these wires *after* the lamp has already been booted up.


## When you only have one GND pin on your USB Adapter

If your USB Adapter does not have multiple `GND` pins, then you'll have to find another way to
attach `GPIO0` to ground. Some options:

- **Use a breadbord**, so you can connect the USB Adapter `GND` pin to a row on the bread bord, and
  connect the `GND` and `GPIO0` wires of the lamp's board to that same row. The rest of this guide
  will show this method.
  [View example by @mmakaay](../images/13_connect_to_serial_to_usb_adapter.jpg).

- **Solder a button on the board** that connects `GPIO0` to `GND` when pressed. Then you can hold
  down this button while plugging in the lamp's power supply. After booting up, you can release the
  button (the serial console will also mention that flash mode is now enabled). This is not the most
  practical solution for most people (since only one such flash operation is needed, from then on
  OTA - Over The Air - updates are possible), but it was a great help to me during the initial
  reverse engineering and firmware development. Some example implementations:
  [a crude one by @mmakaay](../images/13_connect_to_serial_with_button.jpg),
  [one by @edwinschoonhoven](../images/13_connect_to_serial_with_button_alternative.jpg) and
  [one by @mmakaay, inspired by Erwin's](../images/13_connect_to_serial_with_button_alternative2.jpg).

- **Manually hold a wire connected** to both a GND surface (e.g. the silver pad on the left of the
  board) and the `GPIO0` debug pad, while plugging in the power supply. After booting, the wire can
  be removed. This is very fiddly way of doing it (a third hand would be very welcome with this),
  but it can be done.

- **Temporarily solder a lead between `GND` and `GPIO0` on the board**, making `GPIO0` pulled to
  ground permanently. It is a bit less flexible than some other options, but if you only need to do
  the initial backup and firmware flash of the lamp, then this can be all that you need. Remove the
  lead after flashing is done, otherwise the lamp won't boot in normal mode.
  [View example by @erwinschoonhoven](../images/13_connect_to_serial_with_soldered_gnd.jpg).

In the following images, you will see the first solution, using a breadboard.

<img src="../images/13_connect_to_serial_to_usb_adapter.jpg" width="400">

In close up:

<img src="../images/14_connect_to_serial_to_usb_adapter_close_up.jpg" width="400">

You can now connect the serial to USB adapter to you computer. Pay special attention to the
cross-over of the TX/RX pair (TX connects to RX and vice versa). Start the
[esphome-flasher tool](https://github.com/esphome/esphome-flasher) and select the COM port to use.
Then click on "View logs".

Now, plug in the lamp's power supply to boot up the lamp.

<img src="../images/15_power_up_for_flashing.jpg" width="400">

Because GPIO0 is connected to GND, the device should start up in flashing mode. If all went well,
the log output in esphome-flasher looks somewhat like this:

<img src="../images/16_serial_showing_download_mode.png" width="400">


## Make a backup of the current firmware

Backing up the firmware makes it possible to revert to the original firmware, in case you have
problems with the ESPHome firmware. The backup can be created using "esptool". Installation
instructures can be found here:

  https://github.com/espressif/esptool/blob/master/README.md#installation--dependencies

Here's an example on how to backup the original firmware from Linux. First, unplug your lamp's
power supply, then start the esptool read_flash command:

```
python esptool.py -p /dev/ttyUSB0 read_flash 0x0 0x400000 original-firmware.bin
```

`/dev/ttyUSB0` is the port of the USB adaper on Linux. You can find what port is used by the adapter
by running `dmesg` after plugging in the USB device. On Windows this is often `COM1`, `COM2` or
`COM3`.

Now plug back in the power supply. The output of esptool should now show that it connects to the
lamp and downloads the firmware from it.

**Caution**: You will find the WLAN SSID and Password of the last used WiFi network in this file.
Therefore, keep this backup in a safe place.


## How to restore the backed up firmware

In case you need to rollback to the lamp's original firmware at some point, here's an example of how
to restore the original firmware from Windows, by fully flashing it back onto the lamp.

First, unplug your lamp's power supply, then start the esptool write_flash command:

```
python.exe .\esptool.py --chip esp32 --port COM3 --baud 115200 write_flash 0x00 original-firmware.bin
```

Make sure that `GPIO0` is connected to GND and plug in the power supply. The output of esptool
should now show that it connects to the lamp and uploads the firmware to it.

Be patient after the upload reaches 100%. The output is silent while esptool tool is verifying that
the firmware was uploaded correctly.

After the firmware upload completes, unplug the power, disconnect `GPIO0` from GND and reconnect the
power supply to boot into the restored firmware.


## Flash new ESPHome firmware

Setup an ESPHome Project (see [README.md](../README.md)), compile the firmware for the lamp and
download the `firmware.bin` file to the device to which the serial adapter is connected.

You can flash the lamp using esphome or esptool. I would strongly recommend using the
[esphome-flasher](https://github.com/esphome/esphome-flasher) tool. This is a very easy to use GUI
utility app for flashing ESPHome devices and for viewing serial console logging.

- In the app, select the COM port of your serial adapter.
- Then select the firmware.bin file to flash onto the lamp.
- Power up the lamp with `GPIO0` connected to GND.
- Click the "Flash ESP" button to flash the firmware.

If all went well, the final log output in esphome-flasher looks somewhat like this:

<img src="../images/17_flash_ready.png" width="400">

If you want to flash with esptool, you can use the following command.

*Note: unless you know exactly what you're doing with esptool here, I recommend to use the
esphome-flasher instead.*

```
python esptool.py --chip esp32  -p /dev/ttyUSB0 --baud 115200 \
    write_flash -z --flash_mode dout --flash_freq 40m --flash_size detect \
    0x1000 bootloader_dout_40m.bin \
    0x8000 partitions.bin \
    0xe000 boot_app0.bin \
    0x10000 firmware.bin
```

The required .bin files can be found in the following locations:

- **bootloader_dout_40m.bin**: [from arduino-esp32 package](https://github.com/mmakaay/arduino-esp32-unicore-no-mac-crc/blob/v1.0.6/tools/sdk/bin/bootloader_dout_40m.bin) in `tools/sdk/bin/`
- **partitions.bin**: from `<config dir>/<device name>/.pioenvs/<device name>/partitions.bin`
- **boot_app0.bin**: [from arduino-esp32 package](https://github.com/mmakaay/arduino-esp32-unicore-no-mac-crc/blob/v1.0.6/tools/partitions/boot_app0.bin) in `tools/partitions/`
- **firmware.bin**: from `<config dir>/<device name>/.pioenvs/<device name>/firmware.bin`

After flashing, power down the lamp, disconnect `GPIO0` from GND and reconnect the power to boot
into the new ESPHome firmware.

<img src="../images/18_disconnect_GPIO0.jpg" width="200">

The lamp should now be operational using the new firmware.

<img src="../images/19_test_run.jpg" width="200">

From here on, it is possible to flash the lamp OTA (over the air, which means that the firmware is
uploaded over WiFi) from ESPHome. Therefore, it is now time to tuck away or remove those soldered
wires.

Because I want to keep them around for future use, I tuck them away, making sure that the connectors
don't touch each other or the board.

<img src="../images/20_tuck_away_wires.jpg" width="200">

The bottom cover can now be put back on. The lamp is ready for use.

<img src="../images/21_reassemble_and_enjoy.jpg" width="200">


## Troubleshooting flash

If you have **A fatal error occurred: MD5 of file does not match data in flash!**, then make sure
you are powering the board using the lamp's own power adapter. We've seen these errors when trying
to power the board using the 3.3V debug pad.

After seeing this error, user @tabacha was able to successfully flash his lamp using the regular
power adapter.

< [Configuration guide](configuration.md) | [Index](../README.md) | [Technical details](technical_details.md) >
