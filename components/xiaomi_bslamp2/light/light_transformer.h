#pragma once

#include "../common.h"
#include "../light_hal.h" 
#include "color_instant_handler.h"
#include "gpio_outputs.h"
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

  bool is_completed() override {
      return is_completed_ || get_progress_() >= 1.0f;
  }

  void start() override {
    // When a fresh transition is started, then compute the GPIO outputs
    // to use for both the start and end point. This light transition transformer
    // will then transition linearly between these two.
    start_->set_light_color_values(start_values_);
    target_->set_light_color_values(target_values_);

    // When a transition is modified, then use the current GPIO outputs
    // as the new starting point.
    // ... TODO
    //
    ESP_LOGE("DEBUG", "Start this thing"); // TODO
  }

  void finish() override {
    ESP_LOGE("DEBUG", "Finish this thing"); // TODO
  }

  void abort() override {
    ESP_LOGE("DEBUG", "Abort this thing"); // TODO
  }

  optional<light::LightColorValues> apply() override { 
    if (target_->light_mode == "night") {
      ESP_LOGE("DEBUG", "Set night light directly"); // DEBUG
      light_->set_rgbw(target_->red, target_->green, target_->blue, target_->white);
      is_completed_ = true;
    }
    else {
      auto smoothed = light::LightTransitionTransformer::smoothed_progress(get_progress_());
      light_->set_rgbw(
        esphome::lerp(smoothed, start_->red, target_->red),
        esphome::lerp(smoothed, start_->green, target_->green),
        esphome::lerp(smoothed, start_->blue, target_->blue),
        esphome::lerp(smoothed, start_->white, target_->white));
    }

    if (is_completed()) {
      ESP_LOGE("DEBUG", "We're done, publish target values"); // TODO
      is_completed_ = true;
      return target_values_;
    } else {
      return {};
    }
  }

 protected:
  LightHAL *light_;
  bool is_completed_ = false;
  GPIOOutputs *start_ = new ColorInstantHandler();
  GPIOOutputs *target_ = new ColorInstantHandler();
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
