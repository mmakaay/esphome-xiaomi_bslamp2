#pragma once 

#include <cmath>
#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * A text sensor, used for propagating the active light mode on the
 * Yeelight Bedside Lamp 2.
 *
 * The possible light modes are "off", "rgb", "white" and "night".
 */
class YeelightBS2LightModeTextSensor : public text_sensor::TextSensor, public Component {
public:
    void set_parent(YeelightBS2LightOutput *light) { light_ = light; }

    void setup() {
        light_->add_on_state_callback(
            [this](light::LightColorValues values, std::string light_mode) {
                if (last_light_mode_ != light_mode) {
                    publish_state(light_mode);
                    last_light_mode_ = light_mode;
                }
            }
        );
    }

protected:
    YeelightBS2LightOutput *light_;
    std::string last_light_mode_ = LIGHT_MODE_UNKNOWN;
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
