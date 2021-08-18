#pragma once

#include "../light_hal.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This abstract class is used for implementing classes that translate
 * LightColorValues into the required GPIO PWM duty cycle levels to represent
 * the requested color on the physical device.
 */
class ColorHandler : public GPIOOutputValues {
 public:
  /**
   * Sets the red, green, blue, white fields to the PWM duty cycles
   * that are required to represent the requested light color for
   * the provided LightColorValues input.
   *
   * Returns true when the input can be handled, false otherwise.
   */
  virtual bool set_light_color_values(light::LightColorValues v) = 0;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
