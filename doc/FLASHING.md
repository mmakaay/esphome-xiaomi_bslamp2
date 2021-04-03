
# Flahsing Bedlight

## Tools needed

* imbus or torx  screw driver (3mm?)
* Soldering Iron
* pehabs some Sticky tape
* a serial to USB adapter with 3,3 Volt output
* Some wires

## Warning

We have writen this instructions with care, but we will gave absoultly no warnenty. Perhabs you will destroy your device and your computer.


## Opening the yeelight

Remove the rubber on the botton of the bottom of the light.

Unbolt the 4 Screws which was hidden by the rubber.
  ![Photo of the screws](Yeelight_screws.jpg "You will need an imbus or torx screw driver to remove the screws.")

Remove the plastic.

## Solder the wires

Solder the wires to the points of this picture.
  ![Soldering points of a yeelight](https://community-assets.home-assistant.io/optimized/3X/1/3/1340a2367e5894281ac6d042f328d9db80ae7da4_2_790x750.jpeg)

  You can use stick tape to fix the cables before soldering.

## Connect to your flashing device
Make sure that your that your flashing device is given 3.3 Volt between GND and VCC. If not you can use the regular power adapter instead.

  | Soldering point| Serial USB Adapter name  |
  | -------------- |:------------------------:|
  | Gnd            |  Gnd                     |
  | TX             |  RX                      |
  | RX             |  TX                      |
  | GPIO0          |  Gnd                     |
  | 3.3 V          |  VCC or regular power adapter|

Connect all wires execpt VCC or Power Adapter, then plugin the power in the last step.

## Download and install esptool

See: https://github.com/espressif/esptool/blob/master/README.md#installation--dependencies

## Make a backup of the current firmware

```
python esptool.py -p /dev/ttyUSB0 read_flash 0x 0x400000 original-yeelight-firmware.bin
```

/dev/ttyUSB0 is the port of the usb adaper on linux you will see it with `dmesg`. On Windows this is often `COM1`, `COM2` or `COM3`

Caution: You will find WLAN SSID and Password of the lasted used Wifi in this file

**After each step with esptool, you have to unplug you power line and connect it again.**

## Flash new firmware

Setup a ESPHome Project, see [README.md](../README.md)

Now you can flash the device directly with esphome or by esptool.

If you want to flash with esptool, compile with esphome and download it.

```
python.exe .\esptool.py --chip esp32 --port COM3 --baud 115200 write_flash 0x1000 <yourfile.bin>
```

One your firmware is flashed, you can flash more OTA via ESPHOME.
## Troubleshooting flash

If you have **A fatal error occurred: MD5 of file does not match data in flash!**
you can try to use the regular power adapter instead of the vcc line.


After this error, user @tabacha  successfully flashed esp with tasmota bootloader with the regular power adapter and the following command:

```
python esptool.py --chip esp32  -p /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dout --flash_freq 40m --flash_size detect 0x1000 bootloader_dout_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 ~/Downloads/schlafzimmerbedlight.bin
```

You will find the missing tasmota boot files here: https://github.com/arendst/Tasmota/tree/firmware/firmware/tasmota32/ESP32_needed_files

User @tabacha was not able to use tasmota with the beedlight.

