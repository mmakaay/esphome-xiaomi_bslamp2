#pragma once

#include "../common.h"
#include "light_modes.h" 
#include "gpio_outputs.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This class can handle the GPIO outputs in case the light of turned off.
 */
class ColorOff : public GPIOOutputs {
public:
    bool set_light_color_values(light::LightColorValues v) {
        light_mode = LIGHT_MODE_OFF;

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

} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
