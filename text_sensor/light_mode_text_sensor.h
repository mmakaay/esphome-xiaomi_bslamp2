#pragma once

#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * A text sensor, used for propagating the active light mode on the
 * Xiaomi Mijia Bedside Lamp 2.
 *
 * The possible light modes are "off", "rgb", "white" and "night".
 */
class XiaomiBslamp2LightModeTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(XiaomiBslamp2LightOutput *light) { light_ = light; }

  void setup() {
    light_->add_on_light_mode_callback([this](std::string light_mode) {
      if (last_light_mode_ != light_mode) {
        publish_state(light_mode);
        last_light_mode_ = light_mode;
      }
    });
  }

 protected:
  XiaomiBslamp2LightOutput *light_;
  std::string last_light_mode_ = LIGHT_MODE_UNKNOWN;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
