#pragma once 

#include "esphome/core/component.h"
#include "esphome/components/ledc/ledc_output.h"
#include "esphome/components/gpio/output/gpio_binary_output.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

class LightHAL : Component {
public:
    void set_red_pin(ledc::LEDCOutput *pin) { red_ = pin; }
    void set_green_pin(ledc::LEDCOutput *pin) { green_ = pin; }
    void set_blue_pin(ledc::LEDCOutput *pin) { blue_ = pin; }
    void set_white_pin(ledc::LEDCOutput *pin) { white_ = pin; }
    void set_master1_pin(gpio::GPIOBinaryOutput *pin) { master1_ = pin; }
    void set_master2_pin(gpio::GPIOBinaryOutput *pin) { master2_ = pin; }

    void turn_on() {
        master1_->turn_on();
        master2_->turn_on();
    }

    void turn_off() {
        master1_->turn_off();
        master2_->turn_off();
    }

    void set_rgbw(float r, float g, float b, float w) {
        red_->set_level(r);
        green_->set_level(g);
        blue_->set_level(b);
        white_->set_level(w);
    }

protected:
    ledc::LEDCOutput *red_;
    ledc::LEDCOutput *green_;
    ledc::LEDCOutput *blue_;
    ledc::LEDCOutput *white_;
    gpio::GPIOBinaryOutput *master1_;
    gpio::GPIOBinaryOutput *master2_;
};

} // namespace bs2
} // namespace yeelight
} // namespace esphome
