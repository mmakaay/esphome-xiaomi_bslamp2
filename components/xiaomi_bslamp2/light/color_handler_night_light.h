#pragma once

#include "../common.h"
#include "../light_hal.h"
#include "color_handler.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This class can handle the GPIO outputs for the night light mode.
 *
 * At the lowest brightness setting, the light will switch to night light
 * mode. In the Yeelight integration in Home Assistant, this feature is
 * exposed trough a separate switch. I have found that the switch is both
 * confusing and made me run into issues when automating the lights.
 * Using the lowest brightness for triggering the night light feels a lot
 * more natural.
 *
 * Note that if a switch is still the preferred way to handle the night
 * light mode toggle, then this still could be implemented through the
 * device's yaml configuration.
 */
class ColorHandlerNightLight : public ColorHandler {
 public:
  void set_color_temperature_calibration(float red, float green, float blue) {
    color_temperature_red_ = clamp(red, 0.0f, 1.0f);
    color_temperature_green_ = clamp(green, 0.0f, 1.0f);
    color_temperature_blue_ = clamp(blue, 0.0f, 1.0f);
  }

  bool set_light_color_values(light::LightColorValues v) {
    light_mode = LIGHT_MODE_NIGHT;

    // Note: I do not check for a brightness at or below 0.01 (1%) here,
    // because the lowest brightness setting from Home Assistant turns
    // up as 0.011765 in here (which is 3/255 and not 1/100).
    if (v.get_brightness() >= 0.012f)
      return false;

    // This night light mode is activated when white light is selected.
    // Based on measurements using the original device firmware, so it
    // matches the night light of the original firmware.
    if (v.get_color_mode() == light::ColorMode::COLOR_TEMPERATURE) {
      red = color_temperature_red_;
      green = color_temperature_green_;
      blue = color_temperature_blue_;
      white = 0.0f;
    }
    // In RGB mode, the selected color is used to give the night light a
    // specific color, instead of the default. This is a nice extra for
    // this firmware, as the original firmware does not support it.
    else {
      red = std::lerp(0.9997f, 0.9680f, v.get_red());
      green = std::lerp(0.9997f, 0.9680f, v.get_green());
      auto blue_scale = (v.get_red() + v.get_green()) / 2.0f;
      auto blue_max = std::lerp(0.9640f, 0.9720f, blue_scale);
      blue = std::lerp(0.9997f, blue_max, v.get_blue());
      white = 0.0f;
    }

    return true;
  }

 protected:
  float color_temperature_red_{0.968f};
  float color_temperature_green_{0.968f};
  float color_temperature_blue_{0.972f};
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
