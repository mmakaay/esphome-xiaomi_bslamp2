#pragma once

#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/output/float_output.h"
#include <cmath>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * An output, used for controlling the front panel illumination and
 * level indicator on the Xiaomi Mijia Bedside Lamp 2 front panel.
 */
class XiaomiBslamp2FrontPanelOutput : public output::FloatOutput, public Component {
 public:
  void set_parent(FrontPanelHAL *front_panel) {
    front_panel_ = front_panel;
  }

  void write_state(float level) {
    front_panel_->set_light_level(level);
  }

  void set_leds(uint16_t leds) {
    front_panel_->set_leds(leds);
  }

  void turn_on_leds(uint16_t leds) {
    front_panel_->turn_on_leds(leds);
  }

  void turn_off_leds(uint16_t leds) {
    front_panel_->turn_off_leds(leds);
  }

 protected:
  FrontPanelHAL *front_panel_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
