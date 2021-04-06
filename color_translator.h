#pragma once

#include <array>
#include <stdexcept>

#include "common.h"
#include "color_off.h"
#include "color_night_light.h"
#include "color_white_light.h"
#include "color_rgb_light.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/// This class translates LightColorValues into GPIO duty cycles
/// for representing a requested light color.
///
/// The code handles all known light modes for the device:
///
/// - off: the light is off
/// - night light: activated when brightness is at its lowest
/// - white light: based on color temperature + brightness
/// - RGB light: based on RGB values + brightness
class ColorTranslator : public GPIOOutputs {
public:
    bool set_light_color_values(light::LightColorValues v) {
        values = v;

        GPIOOutputs *delegate = nullptr;
        
        // Well, not much light here! Use the off "color".
        if (v.get_state() == 0.0f || v.get_brightness() == 0.0f) {
            delegate = off_light_;
        }
        // At the lowest brightness setting, switch to night light mode.
        // In the Yeelight integration in Home Assistant, this feature is
        // exposed trough a separate switch. I have found that the switch
        // is both confusing and made me run into issues when automating
        // the lights.
        // I don't simply check for a brightness at or below 0.01 (1%),
        // because the lowest brightness setting from Home Assistant
        // turns up as 0.011765 in here (which is 3/255).
        else if (v.get_brightness() < 0.012f) {
            delegate = night_light_;
        }
        // When white light is requested, then use the color temperature
        // white light mode: temperature + brightness.
        else if (v.get_white() > 0.0f) {
            delegate = white_light_;
        }
        // Otherwise, use RGB color mode: red, green, blue + brightness.
        else {
            delegate = rgb_light_;
        }

        delegate->set_light_color_values(v);
        delegate->copy_to(this);

        return true;
    }

protected:
    GPIOOutputs *off_light_ = new ColorOff();
    GPIOOutputs *rgb_light_ = new ColorRGBLight();
    GPIOOutputs *white_light_ = new ColorWhiteLight();
    GPIOOutputs *night_light_ = new ColorNightLight();
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
