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
