# Test plan

This is the test plan that I follow for every release. I use it to ensure that no regression has
been introduced.


## Step 1: Preparation - build and flash the firmware

* Use the latest stable release for ESPHome (to make sure that my code builds and works with that
  release). In my clone I make sure that I checked out the master branch and that it's up-to-date
  with the origin.

* Remove the cached framework package: 
  `/bin/rm -fR ~/.platformio/packages/framework-arduinoespressif32` 

* Copy [`example.yaml`](../example.yaml) to a clean build directory, and update the first two sections for the
  local setup (substitutions, wifi, api, ota).

* Use `esphome example.yaml compile` to build the code.

* Upload this code to the lamp.


## Step 2: Perform the tests

* Touch the power button --> the light must turn on.

* Touch the power button again --> the light must turn off.

* Touch the slider at any level --> the light must turn on, using the level at which the slider was
  touched as the brightness. The front panel illumination is turned on and shows the brightness
  level.

* Touch the slider at various points and touch the slider while moving your finger up and down -->
  the brightness of the light and the brightness level on the front panel must follow the touches.

* Touch and hold the power button --> the light must go into "night light" mode. This means that it
  will become very dim. The front panel illumination must be turned off.

* Touch the slider at a random level --> the light must go back to its normal brightness levels and
  the front panel illumination must be turned on again.

* Touch the color button a few times in a row --> the light must loop through the various preset
  light colors.

* Touch and hold the color button --> the light must switch to a different preset group. There are
  two groups: one with rgb colors and one with white light colors, and holding the color button will
  switch between these two.

* Touch the color button to select a light color, long touch it to switch to the other preset group,
  then long touch again to switch the preset group again -> the lamp must remember what light colors
  were active within the preset groups. If the red preset was selected, you switch to the white
  light group, select a different white preset and then go back to the rgb presets, then the light
  must become red again.

* Go to Home Assistant and check:
  * if you can turn on/off the lamp
  * if you can select colors from the RGB color model
  * if you can select colors from the Color Temperature model
  * if you can set the brightness of the lamp
  * if selecting the lowest possible brightness results in the lamp
    switching to "night light" mode
  * if a transition from off to on with a long transition time works (e.g. 10s)
  * if a transition from on to off with a long transition time works
  * if a transition from night light to some bright light (e.g. 10s) can be
    interrupted by a transition back to night light (e.g. after 5 seconds),
    resulting in a transition back (not an instant drop to night light)


## Step 3: Release the new version

Only after performing these tests successfully, the new version can be released.
Release steps are documented in [`releaseplan.md`](releaseplan.md).
