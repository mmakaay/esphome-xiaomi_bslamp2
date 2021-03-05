#pragma once

#include "esphome.h"

#define CONSTANT_BRIGHTNESS true

// The lamp circuitry does not support having RGB and white
// channels active at the same time. Therefore, color interlock
// must be enabled.
#define COLOR_INTERLOCK true

// Same range as supported by the original Yeelight firmware.
#define HOME_ASSISTANT_MIRED_MIN 153
#define HOME_ASSISTANT_MIRED_MAX 588

namespace esphome {
namespace rgbww {

class YeelightBedsideLampV2LightOutput : public Component, public LightOutput
{
  public:
  YeelightBedsideLampV2LightOutput(
      FloatOutput *r, FloatOutput *g, FloatOutput *b, FloatOutput *w,
      esphome::gpio::GPIOBinaryOutput *m1, esphome::gpio::GPIOBinaryOutput *m2) :
    red_(r), green_(g), blue_(b), white_(w), master1_(m1), master2_(m2) {}

  LightTraits get_traits() override
  {
    auto traits = LightTraits();
    traits.set_supports_rgb(true);
    traits.set_supports_color_temperature(true);
    traits.set_supports_brightness(true);
    traits.set_supports_rgb_white_value(false);
    traits.set_supports_color_interlock(COLOR_INTERLOCK);
    traits.set_min_mireds(HOME_ASSISTANT_MIRED_MIN);
    traits.set_max_mireds(HOME_ASSISTANT_MIRED_MAX);
    return traits;
  }

  void write_state(LightState *state) override
  {
    auto values = state->current_values;

    ESP_LOGD("custom", "B = State %f, RGB %f %f %f, BRI %f, TEMP %f",
        values.get_state(),
	values.get_red(), values.get_green(), values.get_blue(),
	values.get_brightness(), values.get_color_temperature());

    // Power down the light when its state is 'off'.
    if (values.get_state() == 0) {
        this->turn_off_();
	return;
    }

    // Leave it to the default tooling to figure out the basics.
    // Because of the color interlocking, there are two possible outcomes:
    // - red, green, blue zero -> the light is in color temperature mode
    // - cwhite, wwhite zero -> the light is in RGB mode
    float red, green, blue, cwhite, wwhite;
    state->current_values_as_rgbww(
        &red, &green, &blue, &cwhite, &wwhite,
	CONSTANT_BRIGHTNESS, COLOR_INTERLOCK);

    if (cwhite > 0 || wwhite > 0) {
        this->turn_on_in_color_temperature_mode_(
	  values.get_color_temperature(), values.get_brightness());
    } else {
        this->turn_on_in_rgb_mode_(
	  values.get_red(), values.get_green(), values.get_blue(), values.get_brightness());
    }
  }

  private:
  FloatOutput *red_;
  FloatOutput *green_;
  FloatOutput *blue_;
  FloatOutput *white_;
  esphome::gpio::GPIOBinaryOutput *master1_;
  esphome::gpio::GPIOBinaryOutput *master2_;

  void turn_off_()
  {
    master1_->turn_off();
    red_->set_level(0);
    green_->set_level(0);
    blue_->set_level(0);
    white_->set_level(0);
  }

  void turn_on_in_rgb_mode_(float red, float green, float blue, float brightness)
  {
    ESP_LOGD("custom", "Activate RGB %f, %f, %f, BRIGHTNESS %f", red, green, blue, brightness);

    // This tunes the power for the red channel a bit, to bring
    // the red=1, green=1, blue=1 color more towards white.
    // (on my lamps, there is a faint hint of red in the color)
    red = red * 0.95f;

    // Compensate for brightness.
    red = red * brightness;
    green = green * brightness;
    blue = blue * brightness;

    // Inverse the signal. The LEDs in the lamp's circuit are brighter
    // when the voltages on the GPIO pins are lower.
    red = 1.0f - red;
    green = 1.0f - green;
    blue = 1.0f - blue;

    float white = 0.0;

    ESP_LOGD("rgb_mode", "LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

    // Drive the LEDs.
    red_->set_level(red);
    green_->set_level(green);
    blue_->set_level(blue);
    white_->set_level(white);
    master1_->turn_on();
  }

  void turn_on_in_color_temperature_mode_(float temperature, float brightness)
  {
    ESP_LOGD("temperature_mode", "Activate TEMPERATURE %f, BRIGHTNESS %f", temperature, brightness);

    // Empirically determined during programming the temperature GPIO output
    // code from below, by checking how far my outputs were off from the
    // original lamp firmeware's outputs. This scaler is used for correcting
    // my output towards the original output.
    float volt_scaler;

    // Temperature band 370 - 588
    if (temperature <= HOME_ASSISTANT_MIRED_MAX && temperature >= 371)
    {
      volt_scaler = 3.23f;

      float start = 371;
      float end = 588;
      float band = end - start;

      float red_volt = 2.86f * (1.0f - brightness);
      float red = red_volt / volt_scaler;

      float green_1 = 2.90f + (temperature - start) * (2.97f - 2.90f) / band;
      float green_100 = 0.45f + (temperature - start) * (1.13f - 0.45f) / band;
      float green_volt = green_1 + brightness * (green_100 - green_1);
      float green = green_volt / volt_scaler;

      float blue = 1.0f;

      float white_1 = 0.28f - (temperature - start) * (0.28f - 0.19f) / band;
      float white_100 = 1.07f - (temperature - start) * (1.07f - 0.22f) / band;
      float white_volt = white_1 + brightness * (white_100 - white_1);
      float white = white_volt / volt_scaler;

      ESP_LOGD("temperature_mode", "LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

      red_->set_level(red);
      green_->set_level(green);
      blue_->set_level(blue);
      white_->set_level(white);
      master2_->turn_on();
      master1_->turn_on();
      return;
    }
    // Temperature band 334 - 370
    else if (temperature >= 334)
    {
      volt_scaler = 3.23f;

      float red_volt = (1.0f - brightness) * 2.86f;
      float red = red_volt / volt_scaler;

      float green_volt = 2.9f - brightness * (2.9f - 0.45f);
      float green = green_volt / volt_scaler;

      float blue = 1.0f;

      float white_volt = 0.28f + brightness * (1.07f - 0.28f);
      float white = white_volt / volt_scaler;

      ESP_LOGD("temperature_mode", "LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

      red_->set_level(red);
      green_->set_level(green);
      blue_->set_level(blue);
      white_->set_level(white);
      master2_->turn_on();
      master1_->turn_on();
      return;
    }
    // Temperature band 313 - 333
    //
    // TODO maybe check if we can git rid of a little bug in the original firmware.
    // The light becomes noticably brighter when moving from temperature 334 to
    // temperature 333. There's a little jump in the lighting output here.
    // Possibly this is a switch from warm to cold lighting as imposed by the
    // LED circuitry, making this unavoidable. However, it would be interesting
    // to see if we can smoothen this out. For now, I'll keep the GPIO output
    // for this bug-by-bug-compatible.
    else if (temperature >= 313)
    {
      volt_scaler = 3.23f;

      float red_volt = 2.89f - brightness * (2.89f - 0.32f);
      float red = red_volt / volt_scaler;

      float green_volt = 2.96f - brightness * (2.96f - 1.03f);
      float green = green_volt / volt_scaler;

      float blue = 1.0f;

      float white_volt = 0.42f + brightness * (2.43f - 0.42f);
      float volt_scaler_white = 3.45f;
      float white = white_volt / volt_scaler_white;

      ESP_LOGD("temperature_mode", "LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

      red_->set_level(red);
      green_->set_level(green);
      blue_->set_level(blue);
      white_->set_level(white);
      master2_->turn_on();
      master1_->turn_on();
      return;
    }
    // Temperature band 251 - 312
    else if (temperature >= 251)
    {
      volt_scaler = 3.48f;

      float red = 1.0f;
      float green = 1.0f;
      float blue = 1.0f;

      float white_volt = 0.5f + brightness * (3.28f - 0.5f);
      float white = white_volt / volt_scaler;

      ESP_LOGD("temperature_mode", "LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

      red_->set_level(red);
      green_->set_level(green);
      blue_->set_level(blue);
      white_->set_level(white);
      master2_->turn_on();
      master1_->turn_on();
      return;
    }
    // Temperature band 223 - 250
    else if (temperature >= 223)
    {
    }
    // Temperature band 153 - 222
    else if (temperature >= HOME_ASSISTANT_MIRED_MIN)
    {
    }

    red_->set_level(0.5);
    green_->set_level(0.5);
    blue_->set_level(0.5);
    master2_->turn_off();
    master1_->turn_off();
  }
};

}  // namespace rgbww
}  // namespace esphome
