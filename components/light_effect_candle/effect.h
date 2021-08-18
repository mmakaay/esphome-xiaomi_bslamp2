#pragma once

#include <utility>
#include "esphome/components/light/light_effect.h"

namespace esphome {
namespace light {

inline static float my_cubic_float() {
  const float r = random_float() * 2.0f - 1.0f;
  return r * r * r;
}

class CandleLightEffect : public LightEffect {
 public:
  explicit CandleLightEffect(const std::string &name) : LightEffect(name) {}

  void apply() override {
    LightColorValues remote = this->state_->remote_values;
    LightColorValues current = this->state_->current_values;
    LightColorValues out;
    const float alpha = this->alpha_;
    const float beta = 1.0f - alpha;
    out.set_state(true);
    out.set_brightness(remote.get_brightness() * beta + current.get_brightness() * alpha +
                       (my_cubic_float() * this->intensity_));
    out.set_red(remote.get_red() * beta + current.get_red() * alpha + (my_cubic_float() * this->intensity_));
    out.set_green(remote.get_green() * beta + current.get_green() * alpha + (my_cubic_float() * this->intensity_));
    out.set_blue(remote.get_blue() * beta + current.get_blue() * alpha + (my_cubic_float() * this->intensity_));
    out.set_white(remote.get_white() * beta + current.get_white() * alpha + (my_cubic_float() * this->intensity_));
    out.set_cold_white(remote.get_cold_white() * beta + current.get_cold_white() * alpha +
                       (my_cubic_float() * this->intensity_));
    out.set_warm_white(remote.get_warm_white() * beta + current.get_warm_white() * alpha +
                       (my_cubic_float() * this->intensity_));

    auto traits = this->state_->get_traits();
    auto call = this->state_->make_call();
    call.set_publish(false);
    call.set_save(false);
    call.set_transition_length_if_supported(0);
    call.from_light_color_values(out);
    call.set_state(true);
    call.perform();
  }

  void set_alpha(float alpha) { this->alpha_ = alpha; }
  void set_intensity(float intensity) { this->intensity_ = intensity; }

 protected:

  float intensity_{};
  float flicker_chance = 0.2f;
  float min_flicker_depth = 0.05f;
  float max_flicker_depth = 0.10f;
  int min_flicker_time = 250;
  int max_flicker_time = 400;
  int max_flickers_ = 5;
};

}  // namespace light
}  // namespace esphome
