#pragma once

#include "../common.h"
#include "../light_hal.h" 
#include "color_handler_chain.h"
#include "esphome/components/light/light_transition.h"
#include "esphome/components/light/transitions.h"
#include "esphome/components/light/light_color_values.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * A LightTransition class for the Xiaomi Mijia Bedside Lamp 2.
 */
class XiaomiBslamp2LightTransition : public light::LightTransition {
 public:
  explicit XiaomiBslamp2LightTransition(std::string name, LightHAL *light) :
      light::LightTransition(name),
      light_(light) { }

  // TODO deprecate or compile for 2018.8.* only.
  bool is_finished() {
      return is_completed();
  }

  bool is_completed() override {
      return force_finish_ || get_progress_() >= 1.0f;
  }

  void start() override {
    // Determine the GPIO outputs to use for the start and end point.
    // This light transition transformer will then transition linearly between them.
    light_->copy_to(start_);
    end_->set_light_color_values(target_values_);

    // Update the light mode of the light HAL to the target state, unless
    // this is night mode. For night mode, the update is done after the
    // state has been reached. This makes sure that forcing instant
    // transitions for night light to night light is only done when the
    // night light status has actually been reached. E.g. when in RGB mode
    // and transitioning to night light in 10 seconds, interrupting this
    // after 5 seconds with a new night light setting should not make the
    // transition instant.
    if (end_->light_mode != LIGHT_MODE_NIGHT) {
      light_->set_light_mode(end_->light_mode);
    }

    // Run callbacks. These are normally called from the LightOutput, but
    // since I don't call LightOutput::write_state() from this transformer's
    // code, these callbacks must be called from this transformer as well.
    light_->do_light_mode_callback(end_->light_mode);
    light_->do_state_callback(target_values_);
  }

  optional<light::LightColorValues> apply() override { 
    // When transitioning between night mode light colors, then do this immediately.
    // The LED driver circuitry is not capable of doing clean color or brightness
    // transitions at the low levels as used for the night light.
    if (end_->light_mode == LIGHT_MODE_NIGHT && start_->light_mode == LIGHT_MODE_NIGHT) {
      light_->set_state(end_);
      force_finish_ = true;
    }
    // Otherwise perform a standard transformation.
    else {
      auto smoothed = smoothed_progress_(get_progress_());
      light_->set_rgbw(
        esphome::lerp(smoothed, start_->red, end_->red),
        esphome::lerp(smoothed, start_->green, end_->green),
        esphome::lerp(smoothed, start_->blue, end_->blue),
        esphome::lerp(smoothed, start_->white, end_->white));
      if (end_->light_mode != LIGHT_MODE_OFF) {
        light_->turn_on();
      }
    }

    if (is_finished()) {
      light_->set_light_mode(end_->light_mode);
      if (end_->light_mode == LIGHT_MODE_OFF) {
          light_->turn_off();
      }

      return target_values_;
    } else {
      return {};
    }
  }

 protected:
  LightHAL *light_;
  bool force_finish_{false};
  GPIOOutputValues *start_ = new GPIOOutputValues();
  ColorHandler *end_ = new ColorHandlerChain();

 protected: 
  static float smoothed_progress_(float x) {
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
  }
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
