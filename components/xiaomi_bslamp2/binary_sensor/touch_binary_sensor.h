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
  void set_parent(FrontPanelHAL *front_panel) { front_panel_ = front_panel; }

  void set_for(int part) { for_ = part; }

  void setup() {
    front_panel_->add_on_event_callback([this](EVENT ev) {
      auto part_in_event = ev & FLAG_PART_MASK;
      if (for_ == 0 || part_in_event == for_) {
        auto new_state = (ev & FLAG_TYPE_MASK) == FLAG_TYPE_TOUCH;
        this->publish_state(new_state);
      }
    });
  }

  void dump_config() {
    ESP_LOGCONFIG(TAG, "Front panel binary_sensor:");
    ESP_LOGCONFIG(TAG, "  For: %s", format_part());
  }

 protected:
  FrontPanelHAL *front_panel_;
  EVENT for_ = 0;

  const char *format_part() {
    switch (for_) {
      case FLAG_PART_POWER:
        return "power button";
      case FLAG_PART_COLOR:
        return "color button";
      case FLAG_PART_SLIDER:
        return "slider";
      default:
        return "ERR";
    }
  }
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
