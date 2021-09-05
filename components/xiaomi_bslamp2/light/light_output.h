#pragma once

#include "../common.h"
#include "../light_hal.h"
#include "color_handler_chain.h"
#include "light_transition.h"
#include "esphome/core/component.h"
#include "esphome/components/ledc/ledc_output.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * A LightOutput class for the Xiaomi Mijia Bedside Lamp 2.
 *
 * The function of this class is to translate a required light state
 * into actual physicial GPIO output signals to drive the device's LED
 * circuitry. It forms the glue between the physical device and the
 * logical light color input.
 */
class XiaomiBslamp2LightOutput : public Component, public light::LightOutput {
 public:
  void set_parent(LightHAL *light) { light_ = light; }

  /**
   * Returns a LightTraits object, which is used to explain to the outside
   * world (e.g. Home Assistant) what features are supported by this device.
   */
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLOR_TEMPERATURE});
    traits.set_min_mireds(MIRED_MIN);
    traits.set_max_mireds(MIRED_MAX);
    return traits;
  }

  /**
   * Applies a requested light state to the physicial GPIO outputs.
   */
  void write_state(light::LightState *state) {
    auto values = state->current_values;

    color_handler_chain->set_light_color_values(values);
    light_->do_light_mode_callback(color_handler_chain->light_mode);
    light_->do_state_callback(values);

    // Note: one might think that it is more logical to turn on the LED
    // circuitry master switch after setting the individual channels,
    // but this is the order that was used by the original firmware. I
    // tried to stay as close as possible to the original behavior, so
    // that's why these GPIOs are turned on at this point.
    if (values.get_state() != 0)
      light_->turn_on();

    // Apply the GPIO output levels as defined by the color handler.
    light_->set_state(color_handler_chain);

    if (values.get_state() == 0)
      light_->turn_off();
  }

 protected:
  LightHAL *light_;
  ColorHandler *color_handler_chain = new ColorHandlerChain();
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
