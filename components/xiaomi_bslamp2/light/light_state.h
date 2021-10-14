#pragma once

#include "../common.h"
#include "interfaces.h"
#include "esphome/components/light/light_state.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

// Can be replaced with light::LightStateRTCState once pull request
// https://github.com/esphome/esphome/pull/1735 is merged.
struct MyLightStateRTCState {
  bool state{false};
  float brightness{1.0f};
  float red{1.0f};
  float green{1.0f};
  float blue{1.0f};
  float white{1.0f};
  float color_temp{1.0f};
  uint32_t effect{0};
};

/**
 * A custom LightState class for the Xiaomi Bedside Lamp 2.
 *
 * It is used by the DiscoAction to apply immediate light output
 * updates, without saving or publishing the new state.
 */
class XiaomiBslamp2LightState : public light::LightState, public LightStateDiscoSupport {
 public:
  XiaomiBslamp2LightState(XiaomiBslamp2LightOutput *output) : light::LightState(output) { }

  void disco_stop() {
    MyLightStateRTCState recovered{};
    if (this->rtc_.load(&recovered)) {
      auto call = make_disco_call(true);
	  call.set_state(recovered.state);
	  call.set_brightness_if_supported(recovered.brightness);
	  call.set_red_if_supported(recovered.red);
	  call.set_green_if_supported(recovered.green);
	  call.set_blue_if_supported(recovered.blue);
	  call.set_white_if_supported(recovered.white);
	  call.set_color_temperature_if_supported(recovered.color_temp);
	  if (recovered.effect != 0) {
		call.set_effect(recovered.effect);
	  } else {
		call.set_transition_length_if_supported(0);
	  }
	  call.perform();
    }
  }

  void disco_apply() {
    this->output_->write_state(this);
    this->next_write_ = false;
  }

  light::LightCall make_disco_call(bool save_and_publish) {
    auto call = this->make_call();
    call.set_save(save_and_publish);
    call.set_publish(save_and_publish);
    return call;
  }
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
