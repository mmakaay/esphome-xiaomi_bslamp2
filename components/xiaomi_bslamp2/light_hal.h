#pragma once

#include "esphome/components/gpio/output/gpio_binary_output.h"
#include "esphome/components/ledc/ledc_output.h"
#include "esphome/core/component.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

static const std::string LIGHT_MODE_UNKNOWN{"unknown"};
static const std::string LIGHT_MODE_OFF{"off"};
static const std::string LIGHT_MODE_RGB{"rgb"};
static const std::string LIGHT_MODE_WHITE{"white"};
static const std::string LIGHT_MODE_NIGHT{"night"};

class GPIOOutputValues {
 public:
  float red = 0.0f;
  float green = 0.0f;
  float blue = 0.0f;
  float white = 0.0f;
  std::string light_mode = LIGHT_MODE_OFF;

  /**
   * Copies the current output values to another GPIOOutputValues object.
   */
  void copy_to(GPIOOutputValues *other) {
    other->red = red;
    other->green = green;
    other->blue = blue;
    other->white = white;
    other->light_mode = light_mode;
  }

  void log(const char *prefix) { ESP_LOGD(TAG, "%s: RGB=[%f,%f,%f], white=%f", prefix, red, green, blue, white); }
};

class LightHAL : public Component, public GPIOOutputValues {
 public:
  void set_red_pin(ledc::LEDCOutput *pin) { red_pin_ = pin; }
  void set_green_pin(ledc::LEDCOutput *pin) { green_pin_ = pin; }
  void set_blue_pin(ledc::LEDCOutput *pin) { blue_pin_ = pin; }
  void set_white_pin(ledc::LEDCOutput *pin) { white_pin_ = pin; }
  void set_master1_pin(gpio::GPIOBinaryOutput *pin) { master1_pin_ = pin; }
  void set_master2_pin(gpio::GPIOBinaryOutput *pin) { master2_pin_ = pin; }

  /**
   * Turn on the master switch for the LEDs.
   */
  void turn_on() {
    master1_pin_->turn_on();
    master2_pin_->turn_on();
    is_on_ = true;
  }
 
  /**
   * Turn off the master switch for the LEDs.
   */
  void turn_off() {
    master1_pin_->turn_off();
    master2_pin_->turn_off();
    is_on_ = false;
  }

  /**
   * Check if the light is turned on.
   */
  bool is_on() {
    return is_on_;
  }

  void set_state(GPIOOutputValues *new_state) {
    new_state->copy_to(this);
    red_pin_->set_level(this->red);
    green_pin_->set_level(this->green);
    blue_pin_->set_level(this->blue);
    white_pin_->set_level(this->white);
  }

  void set_rgbw(float r, float g, float b, float w) {
    red_pin_->set_level(r);
    green_pin_->set_level(g);
    blue_pin_->set_level(b);
    white_pin_->set_level(w);

    this->red = r;
    this->green = g;
    this->blue = b;
    this->white = w;
  }

  void set_light_mode(std::string light_mode) {
    this->light_mode = light_mode;
  }

 protected:
  bool is_on_{false};
  ledc::LEDCOutput *red_pin_;
  ledc::LEDCOutput *green_pin_;
  ledc::LEDCOutput *blue_pin_;
  ledc::LEDCOutput *white_pin_;
  gpio::GPIOBinaryOutput *master1_pin_;
  gpio::GPIOBinaryOutput *master2_pin_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
