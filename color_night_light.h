#pragma once

#include <array>
#include <stdexcept>

#include "common.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

class ColorNightLight : public GPIOOutputs {
public:
    bool set_light_color_values(light::LightColorValues v) {
        values = v;

        // This night light mode is activated when white light is selected.
        // Based on measurements using the original device firmware, so it
        // matches the night light of the original firmware.
        if (v.get_white() > 0) {
            red = 0.968f;
            green = 0.968f;
            blue = 0.972f;
            white = 0.0f;
        }
        // In RGB mode, the selected color is used to give the night light
        // a specific color, instead of the default. This is a nice extra
        // for this firmware, as the original firmware does not support it.
        else {
            red = 0.9997f - v.get_red() * (0.9997f - 0.9680f);
            green = 0.9997f - v.get_green() * (0.9997f - 0.9680f);
            auto blue_on = 0.9720f + (0.9680f - 0.9720f) *  (1.0f - (v.get_red() + v.get_green())/2.0f);
            blue = 0.9997f - v.get_blue() * (0.9997f - blue_on);
            white = 0.0f;
        }

        return true;
    }
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
