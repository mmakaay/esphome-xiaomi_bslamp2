#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "interfaces.h"
#include "light_output.h"
#include "light_state.h"
#include "presets.h"
#include <cmath>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

class BrightnessTrigger : public Trigger<float> {
 public:
  explicit BrightnessTrigger(XiaomiBslamp2LightOutput *parent) {
    parent->add_on_state_callback([this](light::LightColorValues values) {
      auto new_brightness = values.get_brightness();
      if (values.get_state() == 0)
        new_brightness = 0.0f;
      new_brightness = roundf(new_brightness * 100.0f) / 100.0f;
      if (last_brightness_ != new_brightness) {
        trigger(new_brightness);
        last_brightness_ = new_brightness;
      }
    });
  }

 protected:
  float last_brightness_ = -1.0f;
};

template<typename... Ts> class DiscoAction : public Action<Ts...> {
 public:
  explicit DiscoAction(LightStateDiscoSupport *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(bool, disco_state)
  TEMPLATABLE_VALUE(bool, state)
  TEMPLATABLE_VALUE(uint32_t, transition_length)
  TEMPLATABLE_VALUE(uint32_t, flash_length)
  TEMPLATABLE_VALUE(float, brightness)
  TEMPLATABLE_VALUE(float, red)
  TEMPLATABLE_VALUE(float, green)
  TEMPLATABLE_VALUE(float, blue)
  TEMPLATABLE_VALUE(float, color_temperature)
  TEMPLATABLE_VALUE(std::string, effect)

  void play(Ts... x) override {
    if (this->disco_state_.has_value()) {
      auto p = this->disco_state_.optional_value(x...);
      if (!*p) {
        parent_->disco_stop();
        return;
      }
    }

    auto call = this->parent_->make_disco_call(false);
    call.set_state(this->state_.optional_value(x...));
    call.set_brightness(this->brightness_.optional_value(x...));
    call.set_red(this->red_.optional_value(x...));
    call.set_green(this->green_.optional_value(x...));
    call.set_blue(this->blue_.optional_value(x...));
    call.set_color_temperature(this->color_temperature_.optional_value(x...));
    call.set_effect(this->effect_.optional_value(x...));
    call.set_flash_length(this->flash_length_.optional_value(x...));
    call.set_transition_length(this->transition_length_.optional_value(x...));

    // Force the light to update right now, not in the next loop.
    call.perform();
    parent_->disco_apply();
  }

 protected:
  LightStateDiscoSupport *parent_;
};

template<typename... Ts> class ActivatePresetAction : public Action<Ts...> {
 public:
  explicit ActivatePresetAction(PresetsContainer *presets) : presets_(presets) {}

  TEMPLATABLE_VALUE(std::string, operation);
  TEMPLATABLE_VALUE(std::string, group);
  TEMPLATABLE_VALUE(std::string, preset);

  void play(Ts... x) override {
    auto operation = this->operation_.value(x...);

    if (operation == "next_group") {
      presets_->activate_next_group();
    } else if (operation == "next_preset") {
      presets_->activate_next_preset();
    } else if (operation == "activate_group") {
      auto group = this->group_.value(x...);
      presets_->activate_group(group);
    } else if (operation == "activate_preset") {
      auto group = this->group_.value(x...);
      auto preset = this->preset_.value(x...);
      presets_->activate_preset(group, preset);
    }
  }

 protected:
  PresetsContainer *presets_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
