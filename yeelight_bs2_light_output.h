#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ledc/ledc_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/gpio/output/gpio_binary_output.h"


// What seems to be a bug in ESPHome transitioning: when turning on
// the device, the brightness is scaled along with the state (which
// runs from 0 to 1), but when turning off the device, the brightness
// is kept the same while the state goes down from 1 to 0. As a result
// when turning off the lamp with a transition time of 1s, the light
// stays on for 1s and then turn itself off abruptly.
//
// Reported the issue + fix at:
// https://github.com/esphome/esphome/pull/1643
//
// A work-around for this issue can be enabled using the following
// define. Note that the code provides a forward-compatible fix, so
// having this define active with a fixed ESPHome version should
// not be a problem.
#define TRANSITION_TO_OFF_BUGFIX

namespace esphome
{
    namespace rgbww
    {
        static const char *TAG = "yeelight_bs2.light";

        // Same range as supported by the original Yeelight firmware.
        static const int HOME_ASSISTANT_MIRED_MIN = 153;
        static const int HOME_ASSISTANT_MIRED_MAX = 588;
        
        // The PWM frequency as used by the original device
        // for driving the LED circuitry.
        const float PWM_FREQUENCY = 3000.0f;

        class YeelightBS2LightOutput : public Component, public light::LightOutput
        {
        public:
		    void set_red(ledc::LEDCOutput *red) { red_ = red; red_->set_frequency(PWM_FREQUENCY); }
		    void set_green(ledc::LEDCOutput *green) { green_ = green; green_->set_frequency(PWM_FREQUENCY); }
		    void set_blue(ledc::LEDCOutput *blue) { blue_ = blue; blue_->set_frequency(PWM_FREQUENCY); }
		    void set_white(ledc::LEDCOutput *white) { white_ = white; white_->set_frequency(PWM_FREQUENCY); }
            void set_master1(gpio::GPIOBinaryOutput *master1) { master1_ = master1; }
            void set_master2(gpio::GPIOBinaryOutput *master2) { master2_ = master2; }

            light::LightTraits get_traits() override
            {
                auto traits = light::LightTraits();
                traits.set_supports_rgb(true);
                traits.set_supports_color_temperature(true);
                traits.set_supports_brightness(true);
                traits.set_supports_rgb_white_value(false);
                traits.set_supports_color_interlock(true);
                traits.set_min_mireds(HOME_ASSISTANT_MIRED_MIN);
                traits.set_max_mireds(HOME_ASSISTANT_MIRED_MAX);
                return traits;
            }

            void write_state(light::LightState *state) override
            {
                auto values = state->current_values;

                ESP_LOGD(TAG, "B = State %f, RGB %f %f %f, BRI %f, TEMP %f",
                         values.get_state(),
                         values.get_red(), values.get_green(), values.get_blue(),
                         values.get_brightness(), values.get_color_temperature());

                // Power down the light when its state is 'off'.
                if (values.get_state() == 0)
                {
                    this->turn_off_();
#ifdef TRANSITION_TO_OFF_BUGFIX
                    previous_state_ = -1;
                    previous_brightness_ = 0;
#endif
                    return;
                }

                auto brightness = values.get_brightness();

#ifdef TRANSITION_TO_OFF_BUGFIX
                // Remember the brightness that is used when the light is fully ON.
                if (values.get_state() == 1) {
                    previous_brightness_ = brightness;
                }
                // When transitioning towards zero brightness ...
                else if (values.get_state() < previous_state_) {
                    // ... check if the prevous brightness is the same as the current
                    // brightness. If yes, then the brightness isn't being scaled ...
                    if (previous_brightness_ == brightness) {
                        // ... and we need to do that ourselves.
                        brightness = values.get_state() * brightness;
                    }
                }
                previous_state_ = values.get_state();
#endif

                // Leave it to the default tooling to figure out the basics.
                // Because of the color interlocking, there are two possible outcomes:
                // - red, green, blue zero -> the light is in color temperature mode
                // - cwhite, wwhite zero -> the light is in RGB mode
                float red, green, blue, cwhite, wwhite;
                state->current_values_as_rgbww(&red, &green, &blue, &cwhite, &wwhite, true, false);

                if (cwhite > 0 || wwhite > 0)
                {
                    this->turn_on_in_color_temperature_mode_(
                        values.get_color_temperature(), brightness);
                }
                else
                {
                    // The RGB mode does not use the RGB values as determined by
                    // current_values_as_rgbww(). The device has LED driving circuitry
                    // that takes care of the required brightness curve while ramping up
                    // the brightness. Therefore, the actual RGB values are passed here.
                    this->turn_on_in_rgb_mode_(
                        values.get_red(), values.get_green(), values.get_blue(),
                        brightness, values.get_state());
                }
            }

        protected:
            ledc::LEDCOutput *red_;
            ledc::LEDCOutput *green_;
            ledc::LEDCOutput *blue_;
            ledc::LEDCOutput *white_;
            esphome::gpio::GPIOBinaryOutput *master1_;
            esphome::gpio::GPIOBinaryOutput *master2_;
#ifdef TRANSITION_TO_OFF_BUGFIX
            float previous_state_ = 1;
            float previous_brightness_ = -1;
#endif

            void turn_off_()
            {
                red_->set_level(1);
                green_->set_level(1);
                blue_->set_level(1);
                white_->turn_off();
                master2_->turn_off();
                master1_->turn_off();
            }

            void turn_on_in_rgb_mode_(float red, float green, float blue, float brightness, float state)
            {
                ESP_LOGD(TAG, "Activate RGB %f, %f, %f, BRIGHTNESS %f", red, green, blue, brightness);

                // The brightness must be at least 3/100 to light up the LEDs.
                // During transitions (where state is a fraction between 0 and 1,
                // indicating the transition progress) we don't apply this to
                // get smoother transitioning when turning on the light.
                if (state == 1 && brightness < 0.03f)
                    brightness = 0.03f;

                // Apply proper color mixing around the RGB white point.
                // Overall, the RGB colors are very usable when simply scaling the
                // RGB channels with the brightness, but around the white point,
                // the color is a bit on the red side of the spectrum. The following
                // scaling was created to fix that.
                //  RGBW 0.432451, 0.013149, 0.556678
                //  R 0.57 g 1 b 0.45
                auto red_w = (0.07f + brightness*(0.57f - 0.07f)) * red;
                auto green_w = (0.13f + brightness*(1.00f - 0.13f)) * green;
                auto blue_w = (0.06f + brightness*(0.45f - 0.06f)) * blue;

                // For other colors, we can simply scale the RGB channels with the
                // requested brightness, resulting in a very usable color. Not 100%
                // the same as the original firmware, but sometimes even better IMO.
                auto red_c = red * brightness;
                auto green_c = green * brightness;
                auto blue_c = blue * brightness;

                // The actual RGB values are a weighed mix of the above two.
                // The closer to the white point, the more the white point
                // value applies.
                auto level_red = (red_w * ((green+blue)/2)) + (red_c * (1-(green+blue)/2));
                auto level_green = (green_w * ((red+blue)/2)) + (green_c * (1-(red+blue)/2));
                auto level_blue = (blue_w * ((red+green)/2)) + (blue_c * (1-(red+green)/2));

                // Invert the signal. The LEDs in the lamp's circuit are brighter
                // when the pwm levels on the GPIO pins are lower.
                level_red = 1.0f - level_red;
                level_green = 1.0f - level_green;
                level_blue = 1.0f - level_blue;

                ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, off", level_red, level_green, level_blue);

                // Drive the LEDs.
                master2_->turn_on();
                master1_->turn_on();
                red_->set_level(level_red);
                green_->set_level(level_green);
                blue_->set_level(level_blue);
                white_->turn_off();
            }

            void turn_on_in_color_temperature_mode_(float temperature, float brightness)
            {
                ESP_LOGD(TAG, "Activate TEMPERATURE %f, BRIGHTNESS %f", temperature, brightness);

                // Empirically determined during programming the temperature GPIO output
                // code from below, by checking how far my outputs were off from the
                // original lamp firmeware's outputs. This scaler is used for correcting
                // my output towards the original output.
                float scaler;

                float red = 1.0;
                float green = 1.0;
                float blue = 1.0;
                float white = 1.0;

                // Temperature band 370 - 588
                if (temperature <= HOME_ASSISTANT_MIRED_MAX && temperature >= 371)
                {
                    scaler = 3.23f;

                    float start = 371;
                    float end = 588;
                    float band = end - start;

                    float red_volt = 2.86f * (1.0f - brightness);
                    red = red_volt / scaler;

                    float green_1 = 2.90f + (temperature - start) * (2.97f - 2.90f) / band;
                    float green_100 = 0.45f + (temperature - start) * (1.13f - 0.45f) / band;
                    float green_volt = green_1 + brightness * (green_100 - green_1);
                    green = green_volt / scaler;

                    float white_1 = 0.28f - (temperature - start) * (0.28f - 0.19f) / band;
                    float white_100 = 1.07f - (temperature - start) * (1.07f - 0.22f) / band;
                    float white_volt = white_1 + brightness * (white_100 - white_1);
                    white = white_volt / scaler;
                }
                // Temperature band 334 - 370
                else if (temperature >= 334)
                {
                    scaler = 3.23f;

                    float red_volt = (1.0f - brightness) * 2.86f;
                    red = red_volt / scaler;

                    float green_volt = 2.9f - brightness * (2.9f - 0.45f);
                    green = green_volt / scaler;

                    float white_volt = 0.28f + brightness * (1.07f - 0.28f);
                    white = white_volt / scaler;
                }
                // Temperature band 313 - 333
                //
                // The light becomes noticably brighter when moving from temperature 334 to
                // temperature 333. There's a little jump in the lighting output here.
                // Possibly this is a switch from warm to cold lighting as imposed by the
                // LED circuitry, making this unavoidable. However, it would be interesting
                // to see if we can smoothen this out.
                // BTW: This behavior is in sync with the original firmware.
                else if (temperature >= 313)
                {
                    scaler = 3.23f;

                    float red_volt = 2.89f - brightness * (2.89f - 0.32f);
                    red = red_volt / scaler;

                    float green_volt = 2.96f - brightness * (2.96f - 1.03f);
                    green = green_volt / scaler;

                    float white_volt = 0.42f + brightness * (2.43f - 0.42f);
                    float scaler_white = 3.45f;
                    white = white_volt / scaler_white;
                }
                // Temperature band 251 - 312
                else if (temperature >= 251)
                {
                    scaler = 3.48f;

                    float white_correction = 1.061;
                    float white_volt = 0.5f + brightness * (3.28f * white_correction - 0.5f);
                    white = white_volt / scaler;
                }
                // Temperature band 223 - 250
                else if (temperature >= 223)
                {
                    scaler = 3.25f;

                    float green_volt = 2.94f - brightness * (2.94f - 0.88f);
                    green = green_volt / scaler;

                    float blue_volt = 3.02f - brightness * (3.02f - 1.59f);
                    blue = blue_volt / scaler;

                    float white_correction = 1.024f;
                    float white_volt = 0.42f + brightness * (2.51f * white_correction - 0.42f);
                    float scaler_white = 3.36f;
                    white = white_volt / scaler_white;
                }
                // Temperature band 153 - 222
                else if (temperature >= HOME_ASSISTANT_MIRED_MIN)
                {
                    float start = 153;
                    float end = 222;
                    float band = end - start;

                    scaler = 3.23f;

                    float green_volt = 2.86f - brightness * 2.86f;
                    green = green_volt / scaler;

                    float blue_1 = 2.92f + (temperature - start) * (2.97f - 2.92f) / band;
                    float blue_100 = 0.62f + (temperature - start) * (1.17f - 0.62f) / band;
                    float blue_volt = blue_1 - brightness * (blue_1 - blue_100);
                    blue = blue_volt / scaler;

                    float white_1 = 0.28f + (temperature - start) * (0.37f - 0.28f) / band;
                    float white_100 = 1.1f + (temperature - start) * (2.0f - 1.1f) / band;
                    float white_volt = white_1 + brightness * (white_100 - white_1);
                    float scaler_white = 3.27f;
                    white = white_volt / scaler_white;
                }

                ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, %f", red, green, blue, white);

                master2_->turn_on();
                master1_->turn_on();
                red_->set_level(red);
                green_->set_level(green);
                blue_->set_level(blue);
                white_->set_level(white);
            }
        };

    } // namespace rgbww
} // namespace esphome
