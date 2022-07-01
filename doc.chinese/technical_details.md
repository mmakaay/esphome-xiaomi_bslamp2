< [Flashing guide](flashing.md) | [Index](../README.md) | [Sponsoring](sponsoring.md) >

# Technical details

In this section, you can find some of the information that was gathered during the reverse
engineering of the Bedside Lamp 2 hardware.

Table of contents:

* [High level overview](#high-level-overview)
* [ESP32 pinout](#esp32-pinout)
* [Front panel](#front-panel)
* [Build requirements](#build-requirements)
* [Original firmware](#original-firmware)


## High level overview

No documentation is complete without some ASCII art schematics.

```
                                    RX/TX/GND for
   12V power supply                 flashing and logs
         |                               |           
         v                               |                 Front panel
 +---------------+                  +---------------+         .---.
 | Power supply  |---- 3.3V -.----->| ESP-WROOM-32D |<- I2C ->| O | -- color
 +---------------+            \     | single core   |         |   |    button
         |                     \    | 4 MB flash    |<- IRQ --| | |
        12V                     \   +---------------+         | | |
         |                       `-------|------------------->| | | -- slider
         v                               |                    | | |
 +---------------+                       |                    | | |
 | RGB and white |<---- RGBW + master ---+                    |   |    power
 | LED circuitry |      PWM    on/off                         | O | -- button
 +---------------+                                            `---`
```

The LED circuitry provides two light modes:

* Colored RGB light;
* Warm to cool white light.

The front panel of the device contains a two touch buttons (power on/off and color selection) and a
touch slider (for setting the brightness level). This panel is lit when the device is turned on. The
light behind the slider will represent the actual brightness setting of the device.


## ESP32 pinout

In the following image, you can find the pinout as used for the ESP32:

<img src="../images/hardware/ESP32_pinout.jpg" width="600">

Here's an overview of all exposed pins of the chip, starting at the GND + 3.3V pins, and going
anti-clockwise. The table shows not only the functions of the pins that are actually in use by the
lamp's circuitry, but also the pins that are not in use and their possible use.

| PIN  | GPIO#  | Function  | Description                    | Possible use |
|------|--------|-----------|--------------------------------|--------------|
| GND  |        | Ground    | Connected to ground            | -            |
| 3.3V |        | Power     | Power supply input             | -            |
| 9    |        | Reset     | Can be pulled to GND to reset  | -            |
| 5    | GPIO36 | -         |                                | IN           |
| 8    | GPIO39 | -         |                                | IN           |
| 10   | GPIO34 | -         |                                | IN           |
| 11   | GPIO35 | -         |                                | IN           |
| 12   | GPIO32 | -         |                                | IN/OUT       |
| 13   | GPIO33 | LEDs      | LEDs, master switch 1          | -            |
| 14   | GPIO25 | ???       | 10k pull up, unknown function  | IN/OUT (1)   |
| 15   | GPIO26 | -         |                                | IN/OUT       |
| 16   | GPIO27 | -         |                                | IN/OUT       |
| 17   | GPIO14 | LEDs      | LEDs, green PWM channel        | -            |
| 18   | GPIO12 | LEDs      | LEDs, white PWM channel        | -            |
| GND  |        | Ground    | Connected to ground            | -            |
| 20   | GPIO13 | LEDs      | LEDs, red PWM channel          | -            |
| 28   | GPIO9  | SPI       | SPI flash memory               | -            |
| 29   | GPIO10 | SPI       | SPI flash memory               | -            |
| 30   | GPIO11 | SPI       | SPI flash memory               | -            |
| 31   | GPIO6  | SPI       | SPI flash memory               | -            |
| 32   | GPIO7  | SPI       | SPI flash memory               | -            |
| 33   | GPIO8  | SPI       | SPI flash memory               | -            |
| 21   | GPIO15 | -         |                                | IN/OUT (2)   |
| 22   | GPIO2  | -         | Debug pad, no function         | IN/OUT (3)   |
| 23   | GPIO0  | Boot mode | Pull to GND for flashing mode  | -            |
| 24   | GPIO4  | LEDs      | LEDs, master switch 2          | -            |
| 25   | GPIO16 | Front pnl | Front panel interrupt          | -            |
| 27   | GPIO17 | EEPROM    | EEPROM I2C SDA (4)             | -            |
| 34   | GPIO5  | LEDs      | LEDs, blue PWM channel         | -            |
| 35   | GPIO18 | EEPROM    | EEPROM I2C CLK (4)             | -            |
| 38   | GPIO19 | Front pnl | Front panel I2C SCL            | -            |
| N/C  |        |           |                                |              |
| 42   | GPIO21 | Front pnl | Front panel I2C SDA            | -            |
| 40   | GPIO3  | Serial    | Debug pad, RX (flashing, logs) | -            |
| 41   | GPIO1  | Serial    | Debug pad, TX (flashing, logs) | -            |
| 39   | GPIO22 | -         |                                | IN/OUT       |
| 36   | GPIO23 | -         |                                | IN/OUT       |
| GND  |        | Ground    | Connected to ground            | -            |

1. GPIO25 is connected to a 10k pull up resistor. This suggests that it might have some function in
   the lamp, but I have not found that function yet. If you find the actual use for this pin, or
   find that you can indeed repurpose it, then please let me know.
1. Beware that GPIO15 outputs a PWM signal at boot. This might make the pin less useful for your use
   case.
1. Often, GPIO2 is used for an on-board LED. Here, it is only connected to the debug pad. The pin is
   usable for I/O (I tested it), which is great because of the easy access of the debug pad. GPIO2
   might only be used for testing purposes in the original firmware.
1. The connected IC, using I2C address 0x10, looks a lot like an EEPROM, but this has yet to be
   confirmed. It uses a decicated I2C bus, separate from the I2C bus of the front panel.
   [This picture](../images/hardware/IC_on_I2C_GPIO1718.jpg) shows the IC.

For more information on the use of pins on the ESP32 chip, please check out
this [ESP32 pinout reference information](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/).


## Front panel

<img src="../images/hardware/front_panel.jpg" width="150">

The front panel is a stand-alone component, with its own control chip (KungFu KF8TS2716).
Communication between the ESP32 and the front panel is done using:

- **An I2C bus**
  - the front panel is the I2C slave, the ESP32 is the I2C master
    (pardon the standard terminology, I am aware of the controversy)
  - the front panel device ID is 0x2C
  - SDA is connected to ESP32 pin GPIO21
  - SCL is connected to ESP32 pin GPIO19
- **An interrupt data line to signal the ESP32 about new events**
  - this line is connected to ESP32 pin GPIO16
  - the default state is HIGH
  - line is pulled LOW for at least 6 ms when a new event is available

Commands can be written to and data can be read from the front panel component using I2C. The I2C
protocol is fairly simple. All read and write operations uses 6 bytes of data. No register selection
is done before reading or writing.

The interrupt data line is used by the front panel, to signal the ESP32 that a new button or slider
event is available. Further details on this can be found below.

**Connection to the main board**

The front panel is connected to the main board using a flat cable. The picture below shows the
connector on the main board, including the functions of the cable pins:

<img src="../images/hardware/front_panel_flat_cable_connection.jpg" width="400">

**Writing commands to the front panel**

Commands can be written to the front panel at any time.

The commands that are used by the original firmware are these:

| Command         | Byte sequence to send |
|-----------------|-----------------------|
| TURN PANEL ON   | 02 03 5E 00 64 00 00  |
| TURN PANEL OFF  | 02 03 0C 00 64 00 00  |
| SET LEVEL 1     | 02 03 5E 00 64 00 00  |
| SET LEVEL 2     | 02 03 5F 00 64 00 00  |
| SET LEVEL 3     | 02 03 5F 80 64 00 00  |
| SET LEVEL 4     | 02 03 5F C0 64 00 00  |
| SET LEVEL 5     | 02 03 5F E0 64 00 00  |
| SET LEVEL 6     | 02 03 5F F0 64 00 00  |
| SET LEVEL 7     | 02 03 5F F8 64 00 00  |
| SET LEVEL 8     | 02 03 5F FC 64 00 00  |
| SET LEVEL 9     | 02 03 5F FE 64 00 00  |
| SET LEVEL 10    | 02 03 5F FF 64 00 00  |
| READY FOR EVENT | 01 00 00 00 00 00 01  |

*Note: The `READY FOR EVENT` command is only used when a new event is provided by the front panel.
Information about this command can be found in the next section.*

Further experimentation has uncovered that the LEDs of the front panel can be controlled
individually. The original firmware does not use this feature, but I built support for it
into the custom firmware, because it opens up some nice possibilities.

How this works, is that the general format of the "set LEDs" command is: `02 03 XX XX 64 00 00`.
The LEDs to enable are specified using the `XX XX` part. This is a 16 bit value, which can be
constructed by bitwise OR-ing the following LED bit values:

| LED to enable | Bit pattern       |
|---------------|-------------------|
| POWER         | 01001100 00000000 |
| COLOR         | 00011100 00000000 |
| LED 1         | 00001110 00000000 |
| LED 2         | 00001101 00000000 |
| LED 3         | 00001100 10000000 |
| LED 4         | 00001100 01000000 |
| LED 5         | 00001100 00100000 |
| LED 6         | 00001100 00010000 |
| LED 7         | 00001100 00001000 |
| LED 8         | 00001100 00000100 |
| LED 9         | 00001100 00000010 |
| LED 10        | 00001100 00000001 |

LED 1 is the one closest to the power button.
LED 10 is the one closest to the color button.

**Reading events from the front panel**

The types of events that can occur can be summarized as:

- Touch or release the power button
- Touch or release the color button
- Touch or release the slider at a certain level

Because the front panel is an I2C slave device, it cannot contact the ESP32 via I2C. Only an I2C
master device can initiate communication. Therefore, when the front panel has a new event available,
it will pull down the interrupt line for a short period of time, to signal the ESP32 about this new
event.

*Note that the ESP32 needs to poll the interrupt line at least at 667 Hz to be able to trustworthy
detect the 6 ms signal. Unfortunately, the interrupt line does not wait for the ESP32 to respond to
its signalling. The best way to handle signals from this line, is to use an actual interrupt
handler.*

After detecting this signal, the ESP32 must first write the "READY FOR EVENT" command (`01 00 00 00
00 00 01`) via I2C to the front panel.

After the front panel has ACK'ed this command, the ESP32 can read 6 bytes, which will represent the
event that occurred.

Here's the mapping for the events and their corresponding byte sequences:

|                 | Touch event          | Release event        |
|-----------------|----------------------|----------------------|
| POWER BUTTON    | 04 04 01 00 01 01 03 | 04 04 01 00 01 02 04 |
| COLOR BUTTON    | 04 04 01 00 02 01 04 | 04 04 01 00 02 02 05 |
| SLIDER LEVEL 1  | 04 04 01 00 03 16 1A | 04 04 01 00 04 16 1B |
| SLIDER LEVEL 2  | 04 04 01 00 03 15 19 | 04 04 01 00 04 15 1A |
| SLIDER LEVEL 3  | 04 04 01 00 03 14 18 | 04 04 01 00 04 14 19 |
| SLIDER LEVEL 4  | 04 04 01 00 03 13 17 | 04 04 01 00 04 13 18 |
| SLIDER LEVEL 5  | 04 04 01 00 03 12 16 | 04 04 01 00 04 12 17 |
| SLIDER LEVEL 6  | 04 04 01 00 03 11 15 | 04 04 01 00 04 11 16 |
| SLIDER LEVEL 7  | 04 04 01 00 03 10 14 | 04 04 01 00 04 10 15 |
| SLIDER LEVEL 8  | 04 04 01 00 03 0F 13 | 04 04 01 00 04 0F 14 |
| SLIDER LEVEL 9  | 04 04 01 00 03 0E 12 | 04 04 01 00 04 0E 13 |
| SLIDER LEVEL 10 | 04 04 01 00 03 0D 11 | 04 04 01 00 04 0D 12 |
| SLIDER LEVEL 11 | 04 04 01 00 03 0C 10 | 04 04 01 00 04 0C 11 |
| SLIDER LEVEL 12 | 04 04 01 00 03 0B 0F | 04 04 01 00 04 0B 10 |
| SLIDER LEVEL 13 | 04 04 01 00 03 0A 0E | 04 04 01 00 04 0A 0F |
| SLIDER LEVEL 14 | 04 04 01 00 03 09 0D | 04 04 01 00 04 09 0E |
| SLIDER LEVEL 15 | 04 04 01 00 03 08 0C | 04 04 01 00 04 08 0D |
| SLIDER LEVEL 16 | 04 04 01 00 03 07 0B | 04 04 01 00 04 07 0C |
| SLIDER LEVEL 17 | 04 04 01 00 03 06 0A | 04 04 01 00 04 06 0B |
| SLIDER LEVEL 18 | 04 04 01 00 03 05 09 | 04 04 01 00 04 05 0A |
| SLIDER LEVEL 19 | 04 04 01 00 03 04 08 | 04 04 01 00 04 04 09 |
| SLIDER LEVEL 20 | 04 04 01 00 03 03 07 | 04 04 01 00 04 03 08 |
| SLIDER LEVEL 21 | 04 04 01 00 03 02 06 | 04 04 01 00 04 02 07 |
| SLIDER LEVEL 22 | 04 04 01 00 03 01 05 | 04 04 01 00 04 01 06 |

**Behavior when more events come in than can be handled**

The front panel does not queue events. When a new event occurs, before the previous event has be
read by the ESP32, the new event will replace the old event and a new signal is sent over the
interrupt line.

The ESP32 can read the last event multiple times. It will not be cleared by the front panel after
reading it.


## Build requirements

The ESP-WROOM-32D that is used for this lamp (and for various other Xiaomi devices), contains a
single core CPU, even though the data sheet for ESP-WROOM-32D specifies a dual core CPU. Therefore,
when flashing the device with a generic ESP32 build, you will end up with the following boot error:

```
E (459) cpu_start: Running on single core chip, but application is built with dual core support.
E (459) cpu_start: Please enable CONFIG_FREERTOS_UNICORE option in menuconfig.
```

Another issue with a lot of these devices, is that the MAC address that is burnt into EFUSE does not
match the CRC checksum that is also burnt into EFUSE. Using a generic ESP32 build, you will end up
with the boot error:

```
Base MAC address from BLK0 of EFUSE CRC error
```

For these reasons, you must build the firmware using a taylored version of arduino-esp32.
You can make use of the version [created by @pauln](https://github.com/pauln/arduino-esp32)
or the version [created by @mmakaay](https://github.com/mmakaay/arduino-esp32-unicore-no-mac-crc).

To make use of one of these in an ESPHome build, you'll have to provide a platform package
definition for the PlatformIO build. Here's an example configuration that will work for these Xiaomi
devices:

```yaml
esphome:
  name: my_device_name
  platform: ESP32
  board: esp32doit-devkit-v1
  platformio_options:
    platform: espressif32@3.2.0
    platform_packages: |-
      framework-arduinoespressif32 @ https://github.com/mmakaay/arduino-esp32-unicore-no-mac-crc
```

If you want to build your own platform package, then you can checkout
the build scripts [by @mmakaay here](https://github.com/mmakaay/arduino-esp32-unicore-no-mac-crc-builder).


## Original firmware

Below, I have gathered some of the interesting boot messages from the original firmware. These
messages are logged via the serial interface.

**SPI Flash memory:**
```
boot: SPI Flash RID  : 0xB20B4
boot: SPI Flash  MF  : 0xB4
boot: SPI Flash  ID  : 0x200B
boot: SPI Speed      : 40MHz
boot: SPI Mode       : DIO
boot: SPI Flash Size : 4MB
```

**Partition table:**
```
boot: Partition Table:
boot: ## Label            Usage          Type ST Offset   Length
boot:  0 nvs              WiFi data        01 02 00009000 00004000
boot:  1 otadata          OTA data         01 00 0000d000 00002000
boot:  2 phy_init         RF data          01 01 0000f000 00001000
boot:  3 miio_fw1         OTA app          00 10 00010000 001e0000
boot:  4 miio_fw2         OTA app          00 11 001f0000 001e0000
boot:  5 test             test app         00 20 003d0000 00013000
boot:  6 mfi_p            Unknown data     01 82 003e3000 00001000
boot:  7 factory_nvs      WiFi data        01 02 003e4000 00004000
boot:  8 coredump         Unknown data     01 03 003e8000 00010000
boot:  9 minvs            Unknown data     01 fe 003f8000 00004000
boot: End of partition table
```

**MIIO initialization:**
```
_|      _|  _|_|_|  _|_|_|    _|_|  
_|_|  _|_|    _|      _|    _|    _|
_|  _|  _|    _|      _|    _|    _|
_|      _|    _|      _|    _|    _|
_|      _|  _|_|_|  _|_|_|    _|_|  
08:00:00.200 [I] did=332985470 hostname=MiBedsideLamp2-7651

JENKINS BUILD NUMBER: N/A
BUILD TIME: Sep  5 2019,07:12:39
BUILT BY: N/A
MIIO APP VER: 2.0.6_0030
Setup ID: 95XJ
Getting setup info from factory NVS
MIIO MCU VER: 
MIIO DID: *********
MIIO WIFI MAC: ************
MIIO MODEL: yeelink.light.bslamp2
ARCH TYPE: esp32,0x0000a601
ARCH VER: d178b9b
```

**Network initialized:**
```
[20:27:05]08:00:04.180 [I] miio_net: Wifi station connected
[20:27:05]Registering HomeKit web handlers
[20:27:05]Announcing _hap._tcp mDNS service
```

**Phoning home to the Mijia cloud:**
```
ots: httpdns resolve start failed, -12 (ots_cloud_host_update,850)
otu: Opened.
ots: de.ots.io.mi.com resolved to 3.126.247.75.
ots: ots connect 3.126.247.75::443...
tls: connect to server Mijia Cloud, domain is 3.126.247.75, port is 443.
tls: timeout[100]! mbedtls_ssl_handshake returned -0x6800 (d0_tls_open,369)
tls: timeout[200]! mbedtls_ssl_handshake returned -0x6800 (d0_tls_open,369)
tls: timeout[300]! mbedtls_ssl_handshake returned -0x6800 (d0_tls_open,369)
tls: timeout[400]! mbedtls_ssl_handshake returned -0x6800 (d0_tls_open,369)
ots: Connected.
```


< [Flashing guide](flashing.md) | [Index](../README.md) | [Sponsoring](sponsoring.md) >
