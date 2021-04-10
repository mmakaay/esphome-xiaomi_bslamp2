#pragma once

#include <array>
#include <stdexcept>

#include "../common.h"
#include "gpio_outputs.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * This class can handle the GPIO outputs in case the light of turned off.
 */
class ColorOff : public GPIOOutputs {
protected:
    bool set_light_color_values(light::LightColorValues v) {
        if (v.get_state() != 0.0f && v.get_brightness() != 0.0f) {
            return false;
        }

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
