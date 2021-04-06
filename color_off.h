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
        red   = 0.0f;
        green = 0.0f;
        blue  = 0.0f;
        white = 0.0f;

        return true;
    }
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
