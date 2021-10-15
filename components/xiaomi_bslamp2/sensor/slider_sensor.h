#pragma once

#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/sensor/sensor.h"
#include <cmath>
#include <algorithm>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * A sensor for the touch slider on the front panel of the
 * Xiaomi Mijia Bedside Lamp 2.
 *
 * This sensor publishes the level at which the slider was touched, so it
 * can be used to implement automations. Note that it does not represent
 * the brightness of the LED lights (this is implemented by the light output
 * component), nor the level as displayed by the slider using the front
 * panel illumination (this is implemented by the slider output component).
 */
class XiaomiBslamp2SliderSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(FrontPanelHAL *front_panel) { front_panel_ = front_panel; }
  void set_range_from(float from) { range_from_ = from; }
  void set_range_to(float to) { range_to_ = to; }

  void setup() {
    slope_ = (range_to_ - range_from_) / 19.0f;

    front_panel_->add_on_event_callback([this](EVENT ev) {
      if ((ev & FLAG_PART_MASK) == FLAG_PART_SLIDER) {
        float level = (ev & FLAG_LEVEL_MASK) >> FLAG_LEVEL_SHIFT;

        // Slider level 1 is really hard to touch. It is between
        // the power button and the slider space, so it doesn't
        // look like this one was ever meant to be used, or that
        // the design was faulty on this. Therefore, level 1 is
        // ignored. The resulting range of levels is 0-19.
        float corrected_level = std::max(0.0f, level - 2.0f);

        float final_level = range_from_ + (slope_ * corrected_level);

        // Accomodate for rounding errors that might push the result
        // value just past the "range to" value.
        if (final_level > range_to_) {
            final_level = range_to_;
        }

        this->publish_state(final_level);
      }
    });
  }

  void dump_config() {
    ESP_LOGCONFIG(TAG, "Front panel slider sensor:");
    ESP_LOGCONFIG(TAG, "  Range from: %f", range_from_);
    ESP_LOGCONFIG(TAG, "  Range to: %f", range_to_);
  }

 protected:
  FrontPanelHAL *front_panel_;
  float range_from_;
  float range_to_;
  float slope_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
