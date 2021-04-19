#pragma once 

#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This class implements a binary sensor for the touch buttons
 * and touch slider on the front panel of the Xiaomi Mijia Bedside Lamp 2.
 */
class XiaomiBslamp2TouchBinarySensor : public binary_sensor::BinarySensor, public Component {
public:
    void set_parent(FrontPanelHAL *front_panel) {
        front_panel_ = front_panel;
    }

    void include_part(int part) {
        match_ = match_ | part; 
    }

    void setup() {
        front_panel_->add_on_event_callback(
            [this](EVENT ev) {
                auto part = ev & FLAG_PART_MASK;
                auto is_touch = (ev & FLAG_TYPE_MASK) == FLAG_TYPE_TOUCH;
                if (is_touch) {
                    active_ = active_ | part;
                } else {
                    active_ = active_ & ~part;
                }

                this->publish_state(active_ == match_);
            }
        );
    }

    void dump_config() {
        ESP_LOGCONFIG(TAG, "Front panel binary_sensor:");
        ESP_LOGCONFIG(TAG, "  Part(s):");
        if ((match_ & FLAG_PART_POWER) == FLAG_PART_POWER) {
          ESP_LOGCONFIG(TAG, "    - Power button");
        }
        if ((match_ & FLAG_PART_COLOR) == FLAG_PART_COLOR) {
          ESP_LOGCONFIG(TAG, "    - Color button");
        }
        if ((match_ & FLAG_PART_SLIDER) == FLAG_PART_SLIDER) {
          ESP_LOGCONFIG(TAG, "    - Slider");
        }
    }

protected:
    FrontPanelHAL *front_panel_;
    EVENT match_ = 0;
    EVENT active_ = 0;
};
    
} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
