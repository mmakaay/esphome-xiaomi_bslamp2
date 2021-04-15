#pragma once

#include <array>
#include <stdexcept>

#include "../common.h"
#include "gpio_outputs.h"
#include "color_off.h"
#include "color_night_light.h"
#include "color_white_light.h"
#include "color_rgb_light.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**    
 * This class translates LightColorValues into GPIO duty cycles that
 * can be used for representing a requested light color on the
 * physical device.
 *
 * The code handles all known light modes for the device:
 *
 * - off: the light is off
 * - night light: based on RGB or white mode + lowest possible brightness
 * - white light: based on color temperature + brightness
 * - RGB light: based on RGB values + brightness
 */
class ColorInstantHandler : public GPIOOutputs {
public:
    bool set_light_color_values(light::LightColorValues v) {
        GPIOOutputs *delegate;
        
        // The actual implementation of the various light modes is in
        // separated targeted classes. These classes are called here
        // in a chain of command-like pattern, to let the first one
        // that can handle the light settings do the honours.
        if (off_light_->set_light_color_values(v))
            off_light_->copy_to(this);
        else if (night_light_->set_light_color_values(v))
            night_light_->copy_to(this);
        else if (white_light_->set_light_color_values(v))
            white_light_->copy_to(this);
        else if (rgb_light_->set_light_color_values(v))
            rgb_light_->copy_to(this);
        else 
            throw std::logic_error(
                "None of the GPIOOutputs classes handles the requested light state");

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
