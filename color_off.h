#pragma once

#include <array>
#include <stdexcept>

#include "common.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

class ColorOff : public GPIOOutputs {
public:
    bool set_light_color_values(light::LightColorValues v) {
        // This class can handle the light settings when the light is turned
        // off or the brightness is set to zero.
        if (v.get_state() != 0.0f && v.get_brightness() != 0.0f) {
            return false;
        }

        values = v;

        red   = 1.0f;
        green = 1.0f;
        blue  = 1.0f;
        white = 0.0f;

        return true;
    }
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
