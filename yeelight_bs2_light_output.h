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

// The PWM frequencies as used by the original device
// for driving the LED circuitry.
const float RGB_PWM_FREQUENCY = 3000.0f;
const float WHITE_PWM_FREQUENCY = 9765.0f;

namespace esphome {
namespace rgbww {

    static const char *TAG = "yeelight_bs2.light";

    // Same range as supported by the original Yeelight firmware.
    static const int HOME_ASSISTANT_MIRED_MIN = 153;
    static const int HOME_ASSISTANT_MIRED_MAX = 588;
    
    class YeelightBS2LightOutput : public Component, public light::LightOutput
    {
    public:
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

	    void set_red_output(ledc::LEDCOutput *red) {
            red_ = red;
            red_->set_frequency(RGB_PWM_FREQUENCY);
        }

	    void set_green_output(ledc::LEDCOutput *green) {
            green_ = green;
            green_->set_frequency(RGB_PWM_FREQUENCY);
        }

	    void set_blue_output(ledc::LEDCOutput *blue) {
            blue_ = blue;
            blue_->set_frequency(RGB_PWM_FREQUENCY);
        }

	    void set_white_output(ledc::LEDCOutput *white) {
            white_ = white;
            // Quick fix; when using 10kHz like the original device
            // firmware, the blue channel will use that frequency
            // instead, causing issues in the RGB color settings.
            // This looks like an issue with the ledc component.
            white_->set_frequency(RGB_PWM_FREQUENCY);
        }

        void set_master1_output(gpio::GPIOBinaryOutput *master1) {
            master1_ = master1;
        }

        void set_master2_output(gpio::GPIOBinaryOutput *master2) {
            master2_ = master2;
        }

        void write_state(light::LightState *state) override
        {
            auto values = state->current_values;

            ESP_LOGD(TAG, "write_state: STATE %f, RGB %f %f %f, BRI %f, TEMP %f",
                     values.get_state(),
                     values.get_red(), values.get_green(), values.get_blue(),
                     values.get_brightness(), values.get_color_temperature());

            // Power down the light when its state is 'off'.
            if (values.get_state() == 0)
            {
                turn_off_();
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
                turn_on_in_white_mode_(values.get_color_temperature(), brightness);
            }
            else if (
                values.get_red() == 1 &&
                values.get_green() == 1 &&
                values.get_blue() == 1 &&
                brightness < 0.012f) {
                turn_on_in_night_light_mode_();
            }
            else
            {
                // The RGB mode does not use the RGB values as determined by
                // current_values_as_rgbww(). The device has LED driving circuitry
                // that takes care of the required brightness curve while ramping up
                // the brightness. Therefore, the actual RGB values are passed here.
                turn_on_in_rgb_mode_(
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
        esphome::rgbww::yeelight_bs2::WhiteLight white_light_;
        esphome::rgbww::yeelight_bs2::RGBLight rgb_light_;
        esphome::rgbww::yeelight_bs2::NightLight night_light_;
#ifdef TRANSITION_TO_OFF_BUGFIX
        float previous_state_ = 1;
        float previous_brightness_ = -1;
#endif

        void turn_off_()
        {
            red_->set_level(1);
            green_->set_level(1);
            blue_->set_level(1);
            white_->set_level(0);
            master2_->turn_off();
            master1_->turn_off();
        }

        void turn_on_in_night_light_mode_()
        {
            ESP_LOGD(TAG, "Activate Night light feature");

            night_light_.set_color(1, 1, 1, 0.01, 1);

            ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, %f", night_light_.red, night_light_.green, night_light_.blue, night_light_.white);

            // Drive the LEDs.
            master2_->turn_on();
            master1_->turn_on();
            red_->set_level(night_light_.red);
            green_->set_level(night_light_.green);
            blue_->set_level(night_light_.blue);
            white_->set_level(night_light_.white);
        }

        void turn_on_in_rgb_mode_(float red, float green, float blue, float brightness, float state)
        {
            ESP_LOGD(TAG, "Activate RGB %f, %f, %f, BRIGHTNESS %f", red, green, blue, brightness);

            rgb_light_.set_color(red, green, blue, brightness, state);

            ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, off", rgb_light_.red, rgb_light_.green, rgb_light_.blue);

            // Drive the LEDs.
            master2_->turn_on();
            master1_->turn_on();
            red_->set_level(rgb_light_.red);
            green_->set_level(rgb_light_.green);
            blue_->set_level(rgb_light_.blue);
            white_->turn_off();
        }

        void turn_on_in_white_mode_(float temperature, float brightness)
        {
            ESP_LOGD(TAG, "Activate TEMPERATURE %f, BRIGHTNESS %f",
                temperature, brightness);

            white_light_.set_color(temperature, brightness);

            ESP_LOGD(TAG, "New LED state : RGBW %f, %f, %f, %f", 
                white_light_.red, white_light_.green, white_light_.blue,
                white_light_.white);

            master2_->turn_on();
            master1_->turn_on();
            red_->set_level(white_light_.red);
            green_->set_level(white_light_.green);
            blue_->set_level(white_light_.blue);
            white_->set_level(white_light_.white);
        }
    };

} // namespace rgbww
} // namespace esphome
