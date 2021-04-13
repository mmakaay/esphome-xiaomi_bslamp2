#pragma once 

#include <cmath>
#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * A sensor for the slider on the Yeelight Bedside Lamp 2.
 *
 * This sensor publishes the level at which the slider was touched, so it
 * can be used to implement automations.  Note that it does not represent
 * the brightness of the LED lights (this is implemented by the light output
 * component), nor the level as displayed by the slider using the front
 * panel light (this is implemented by the slider light component).
 */
class YeelightBS2SliderSensor : public sensor::Sensor, public Component {
public:
    void set_parent(FrontPanelHAL *front_panel) {
        front_panel_ = front_panel;
    }

    void setup() {
        ESP_LOGCONFIG(TAG, "Setting up slider sensor ...");

        front_panel_->add_on_event_callback(
            [this](EVENT ev) {
                if ((ev & FLAG_PART_MASK) == FLAG_PART_SLIDER) {
                    auto level = (ev & FLAG_LEVEL_MASK) >> FLAG_LEVEL_SHIFT; 

                    // Slider level 1 is really hard to touch. It is between
                    // the power button and the slider space, so it doesn't
                    // look like this one was ever meant to be used, or that
                    // the design was faulty on this. Therefore, level 1 is
                    // ignored here.
                    level = max(1.0f, level - 1.0f);

                    // Convert the slider level to a float between 0.01 and
                    // 1.00, which is useful as a representation for a
                    // brightness value. The input level is now between
                    // 1 and 20.
                    auto publish_level = max(0.01f, (level-1.0f) * (1.00f / 19.0f));
                    
                    this->publish_state(publish_level); 
                }
            }
        );
    }

protected:
    FrontPanelHAL *front_panel_;
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
