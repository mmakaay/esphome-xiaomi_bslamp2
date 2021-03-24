#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/gpio/output/gpio_binary_output.h"

#define CONSTANT_BRIGHTNESS true

// The lamp circuitry does not support having RGB and white
// channels active at the same time. Therefore, color interlock
// must be enabled.
#define COLOR_INTERLOCK true

// Same range as supported by the original Yeelight firmware.
#define HOME_ASSISTANT_MIRED_MIN 153
#define HOME_ASSISTANT_MIRED_MAX 588

#define TAG "yeelight_bs2"

//#define YEELIGHT_DEBUG_LOG
//#define TRANSITION_TO_OFF_BUGFIX

namespace esphome
{
    namespace rgbww
    {
        class YeelightBS2LightOutput : public Component, public light::LightOutput
        {
        public:
		    void set_red(output::FloatOutput *red) { red_ = red; }
		    void set_green(output::FloatOutput *green) { green_ = green; }
		    void set_blue(output::FloatOutput *blue) { blue_ = blue; }
		    void set_white(output::FloatOutput *white) { white_ = white; }
            void set_master1(gpio::GPIOBinaryOutput *master1) { master1_ = master1; }
            void set_master2(gpio::GPIOBinaryOutput *master2) { master2_ = master2; }

            light::LightTraits get_traits() override
            {
                auto traits = light::LightTraits();
                traits.set_supports_rgb(true);
                traits.set_supports_color_temperature(true);
                traits.set_supports_brightness(true);
                traits.set_supports_rgb_white_value(false);
                traits.set_supports_color_interlock(COLOR_INTERLOCK);
                traits.set_min_mireds(HOME_ASSISTANT_MIRED_MIN);
                traits.set_max_mireds(HOME_ASSISTANT_MIRED_MAX);
                return traits;
            }

            void write_state(light::LightState *state) override
            {
                auto values = state->current_values;

//#ifdef YEELIGHT_DEBUG_LOG
                ESP_LOGD(TAG, "B = State %f, RGB %f %f %f, BRI %f, TEMP %f",
                         values.get_state(),
                         values.get_red(), values.get_green(), values.get_blue(),
                         values.get_brightness(), values.get_color_temperature());
//#endif

                // Power down the light when its state is 'off'.
                if (values.get_state() == 0)
                {
                    this->turn_off_();
                    return;
                }

                auto brightness = values.get_brightness();

#ifdef TRANSITION_TO_OFF_BUGFIX
                // What seems to be a bug in ESPHome transitioning: when turning on
                // the device, the brightness is scaled along with the state (which
                // runs from 0 to 1), but when turning off the device, the brightness
                // is kept the same while the state goes down from 1 to 0. As a result
                // when turning off the lamp with a transition time of 1s, the light
                // stays on for 1s and then turn itself off abruptly.
                // For turning off, I implemented this hack here to make the
                // transition work better.
                if (previous_state_ > values.get_state()) {
                    brightness = values.get_state() * brightness;
                }
                previous_state_ = values.get_state();                
#endif

                // Leave it to the default tooling to figure out the basics.
                // Because of the color interlocking, there are two possible outcomes:
                // - red, green, blue zero -> the light is in color temperature mode
                // - cwhite, wwhite zero -> the light is in RGB mode
                float red, green, blue, cwhite, wwhite;
                state->current_values_as_rgbww(
                    &red, &green, &blue, &cwhite, &wwhite,
                    CONSTANT_BRIGHTNESS, COLOR_INTERLOCK);

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
                        values.get_red(), values.get_green(), values.get_blue(), brightness);
                }
            }

        protected:
            output::FloatOutput *red_;
            output::FloatOutput *green_;
            output::FloatOutput *blue_;
            output::FloatOutput *white_;
            esphome::gpio::GPIOBinaryOutput *master1_;
            esphome::gpio::GPIOBinaryOutput *master2_;
            // Used for a bug hack.
            float previous_state_ = 1;

            void turn_off_()
            {
                // Using set_level() calls for the RGB GPIOs, and not
                // turn_off(), because turn_off() causes some unwanted
                // flashing when powering off at low brightness.
                red_->set_level(1);
                green_->set_level(1);
                blue_->set_level(1);
                white_->turn_off();
                master1_->turn_off();
                master2_->turn_off();
            }

            void turn_on_in_rgb_mode_(float red, float green, float blue, float brightness)
            {
#ifdef YEELIGHT_DEBUG_LOG                
                ESP_LOGD(TAG, "Activate RGB %f, %f, %f, BRIGHTNESS %f", red, green, blue, brightness);
#endif                

                // The brightness must be at least 3/100 to light up the LEDs.
                if (brightness < 0.03f)
                    brightness = 0.03f;

                // Apply brightness.
                red = red * brightness;
                green = green * brightness;
                blue = blue * brightness;

                // Inverse the signal. The LEDs in the lamp's circuit are brighter
                // when the pwm levels on the GPIO pins are lower.
                red = 1.0f - red;
                green = 1.0f - green;
                blue = 1.0f - blue;

#ifdef YEELIGHT_DEBUG_LOG
                ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f", red, green, blue);
#endif                

                // Drive the LEDs.
                red_->set_level(red);
                green_->set_level(green);
                blue_->set_level(blue);
                white_->turn_off();
                master1_->turn_on();
                master2_->turn_on();
            }

            void turn_on_in_color_temperature_mode_(float temperature, float brightness)
            {
#ifdef YEELIGHT_DEBUG_LOG                
                ESP_LOGD(TAG, "Activate TEMPERATURE %f, BRIGHTNESS %f", temperature, brightness);
#endif                

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

#ifdef YEELIGHT_DEBUG_LOG
                ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, %f", red, green, blue, white);
#endif                

                red_->set_level(red);
                green_->set_level(green);
                blue_->set_level(blue);
                white_->set_level(white);
                master2_->turn_on();
                master1_->turn_on();
            }
        };

    } // namespace rgbww
} // namespace esphome
