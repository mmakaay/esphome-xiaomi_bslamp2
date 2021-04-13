#pragma once 

#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * This class implements a binary sensor for the buttons on the
 * Yeelight Bedside Lamp 2.
 */
class YeelightBS2Button : public binary_sensor::BinarySensor, public Component {
public:
    void set_parent(FrontPanelHAL *front_panel) {
        front_panel_ = front_panel;
    }

    void set_part(int part) {
        part_ = part;
    }

    void setup() {
        front_panel_->add_on_event_callback(
            [this](EVENT ev) {
                // Filter events by part, when requested.
                if (part_ > 0) {
                    if ((ev & FLAG_PART_MASK) != (part_ << FLAG_PART_SHIFT)) {
                        return;
                    }
                }
                // Publish the new state, based on the touch/release status..
                auto on_or_off = (ev & FLAG_TYPE_MASK) == FLAG_TYPE_TOUCH;
                this->publish_state(on_or_off); 
            }
        );
    }

    void dump_config() {
        ESP_LOGCONFIG(TAG, "Front panel binary_sensor:");
        ESP_LOGCONFIG(TAG, "  Part: %s (id %d)", 
            (part_ == 1 ? "power button" :
             part_ == 2 ? "color button" :
             part_ == 3 ? "slider" : "any"),
            part_);
    }

protected:
    FrontPanelHAL *front_panel_;
    EVENT part_;
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
