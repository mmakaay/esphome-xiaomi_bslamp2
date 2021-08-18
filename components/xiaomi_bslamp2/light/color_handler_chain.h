#pragma once

#include <array>
#include <stdexcept>

#include "../common.h"
#include "color_handler.h"
#include "color_handler_off.h"
#include "color_handler_rgb.h"
#include "color_handler_color_temperature.h"
#include "color_handler_night_light.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This class translates LightColorValues into GPIO duty cycles that can be
 * used for representing a requested light color on the physical device.
 *
 * The code handles all known light modes for the device:
 *
 * - off: the light is off
 * - night light: based on RGB or white mode + lowest possible brightness
 * - white light: based on color temperature + brightness
 * - RGB light: based on RGB values + brightness
 */
class ColorHandlerChain : public ColorHandler {
 public:
  bool set_light_color_values(light::LightColorValues v) {
    // The actual implementation of the various light modes is in separate
    // targeted classes. These classes are called here in a chain of
    // command-like pattern, to let the first one that can handle the light
    // settings do the honours.
    if (off_light_->set_light_color_values(v))
      off_light_->copy_to(this);
    else if (night_light_->set_light_color_values(v))
      night_light_->copy_to(this);
    else if (white_light_->set_light_color_values(v))
      white_light_->copy_to(this);
    else if (rgb_light_->set_light_color_values(v))
      rgb_light_->copy_to(this);
    else {
      ESP_LOGE(TAG, "Light color error: (None of the ColorHandler classes handles the requested light state; defaulting to 'off'");
      off_light_->copy_to(this);
    }

    return true;
  }

 protected:
  ColorHandler *off_light_ = new ColorHandlerOff();
  ColorHandler *rgb_light_ = new ColorHandlerRGB();
  ColorHandler *white_light_ = new ColorHandlerColorTemperature();
  ColorHandler *night_light_ = new ColorHandlerNightLight();
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
