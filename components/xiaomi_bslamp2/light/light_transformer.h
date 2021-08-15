#pragma once

#include "../common.h"
#include "../light_hal.h" 
#include "color_handler_chain.h"
#include "esphome/components/light/light_transformer.h"
#include "esphome/components/light/light_color_values.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * A LightTransitionTransformer class for the Xiaomi Mijia Bedside Lamp 2.
 */
class XiaomiBslamp2LightTransitionTransformer : public light::LightTransitionTransformer {
 public:
  explicit XiaomiBslamp2LightTransitionTransformer(LightHAL *light) : light_(light) { }

  bool is_finished() override {
      return force_finish_ || get_progress_() >= 1.0f;
  }

  void start() override {
    // Compute the GPIO outputs to use for the end point.
    // This light transition transformer will then transition linearly between
    // the current GPIO outputs and the target ones.
    light_->copy_to(start_);
    end_->set_light_color_values(target_values_);
  }

  optional<light::LightColorValues> apply() override { 
    if (end_->light_mode == "night") {
      light_->set_state(end_);
      force_finish_ = true;
    }
    else {
      auto smoothed = light::LightTransitionTransformer::smoothed_progress(get_progress_());
      light_->set_rgbw(
        esphome::lerp(smoothed, start_->red, end_->red),
        esphome::lerp(smoothed, start_->green, end_->green),
        esphome::lerp(smoothed, start_->blue, end_->blue),
        esphome::lerp(smoothed, start_->white, end_->white));
    }

    if (is_finished()) {
      return target_values_;
    } else {
      return {};
    }
  }

 protected:
  LightHAL *light_;
  bool force_finish_ = false;
  GPIOOutputValues *start_ = new GPIOOutputValues();
  ColorHandler *end_ = new ColorHandlerChain();
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
