[1mdiff --git a/doc/example.yaml b/doc/example.yaml[m
[1mindex 05959b1..919876b 100644[m
[1m--- a/doc/example.yaml[m
[1m+++ b/doc/example.yaml[m
[36m@@ -9,11 +9,11 @@[m [msubstitutions:[m
 [m
   # Derive component identifiers, based on the name.[m
   id_light: ${name}[m
[31m-  id_front_panel_illumination: ${name}_front_panel_output[m
[32m+[m[32m  id_light_mode: ${name}_light_mode[m
   id_power_button: ${name}_power_button[m
   id_color_button: ${name}_color_button[m
   id_slider_level: ${name}_slider_level[m
[31m-  id_light_mode: ${name}_light_mode[m
[32m+[m[32m  id_front_panel_illumination: ${name}_front_panel_illumination[m
 [m
 # --------------------------------------------------------------------------[m
 # Use your own preferences for these components.[m
[36m@@ -32,11 +32,11 @@[m [mcaptive_portal:[m
 [m
 api:[m
   password: "Password-To-Link-HomeAssistant-To-This-Device"[m
[31m-  # Disable the reboot timeout. By default, the lamp reboots[m
[31m-  # after 15 minutes without any client connections (e.g. when[m
[31m-  # home assistant is off line, or when the WiFi is broken).[m
[31m-  # Reboots are annoying though, because the RGBWW LEDs will turn[m
[31m-  # off during the reboot, causing the light to flicker.[m
[32m+[m[32m  # Disable the reboot timeout. By default, the lamp reboots after 15[m
[32m+[m[32m  # minutes without any client connections (e.g. when home assistant is off[m
[32m+[m[32m  # line, or when the WiFi is broken). Reboots are annoying though, because[m
[32m+[m[32m  # the RGBWW LEDs will turn off during the reboot, causing the light to[m
[32m+[m[32m  # flicker.[m
   reboot_timeout: 0s[m
 [m
 ota:[m
[36m@@ -131,9 +131,8 @@[m [mtext_sensor:[m
     id: ${id_light_mode}[m
 [m
 # This float output controls the front panel illumination + level indicator.[m
[31m-# Value 0.0 turns off the illumination.[m
[31m-# Other values (up to 1.0) turn on the illumination and set the level[m
[31m-# indicator to the requested level.[m
[32m+[m[32m# Value 0.0 turns off the illumination. Other values (up to 1.0) turn on[m
[32m+[m[32m# the illumination and set the level indicator to the requested level.[m
 output:[m
   - platform: xiaomi_bslamp2[m
     id: ${id_front_panel_illumination}[m
