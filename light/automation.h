#pragma once

#include <cmath>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "light_output.h"

namespace esphome {
namespace yeelight {
namespace bs2 {
    
class BrightnessTrigger : public Trigger<float> {
public:
    explicit BrightnessTrigger(YeelightBS2LightOutput *parent) {
        parent->add_on_state_callback([this](light::LightColorValues values) {
            auto new_brightness = values.get_brightness();
            if (values.get_state() == 0) {
                new_brightness = 0.0f;
            }
            new_brightness = roundf(new_brightness * 100.0f) / 100.0f;
            if (last_brightness_ != new_brightness) {
                trigger(new_brightness);
                last_brightness_ = new_brightness;
            }
        });
    }
protected:
    float last_brightness_ = -1.0f;
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
