[Index](../README.md) | [Installation guide](installation.md) >

# Why custom ESPHome firmware?

This lamp could always be added to Home Assistant using the Yeelight integration. For this to work,
the lamp had to be configured to enable a special "LAN control" option, which provides the interface
that the Yeelight integration could talk to.

January 2021, Xiaomi decided to remove the LAN control option from the firmware. Information about
this can be found on the Yeelight forums:

  https://forum.yeelight.com/t/topic/22664

In the forums, a work-around is offered: by providing your Mi ID, you can be added to some mythical
white list, through which you'll get a special firmware upgrade that does have LAN control enabled.
After that, no further ugprades should be done.

This just didn't feel right, so I started looking into alternative solutions to regain control over
the lamp. After opening it up, I was pleased to find an ESP32-WROOM-32D chip inside, meaning that it
might be possible to build my own firmware for it using ESPHome.

On the Home Assistant community forums, I found an existing thread where people already started
poking at the lamp.

  https://community.home-assistant.io/t/hacking-yeelight-fw-enabling-lan-control/284406

I joined the discussion and started poking as well. Some things turned out to be more complicated
than anticipated and quite a bit of reverse engineering was required to make things work, but
eventually this all resulted in working firmware.

Documents related to the reverse engineering process can be found in a separate GitHub repository:

  https://github.com/mmakaay/esphome-yeelight_bs2-revengineering

[Index](../README.md) | [Installation guide](installation.md) >
