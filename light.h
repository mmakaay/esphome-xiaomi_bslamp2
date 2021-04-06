#pragma once 

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

namespace esphome {
namespace yeelight {
namespace bs2 {

    /// This is an interface definition that is used to extend the
    /// YeelightBS2LightOutput class with methods to access properties
    /// of an active LightTranformer from the YeelightBS2LightOutput
    /// class.
    /// The transformer is protected in the light output class, making
    /// it impossible to access these properties directly from the
    /// light output class.
    class LightStateDataExposer {
    public:
        virtual bool has_active_transformer() = 0;
        virtual bool transformer_is_transition() = 0;
        virtual light::LightColorValues get_transformer_values() = 0;
        virtual light::LightColorValues get_transformer_end_values() = 0;
        virtual float get_transformer_progress() = 0;
    };

    /// This class translates LightColorValues into GPIO duty cycles
    /// for representing a requested light color.
    class ColorTranslator : public DutyCycles {
    public:
        void set_light_color_values(light::LightColorValues values) {
            // The light is turned off.
            if (values.get_state() == 0.0f || values.get_brightness() == 0.0f) {
                red = 0.0f;
                green = 0.0f;
                blue = 0.0f;
                white = 0.0f;
                return;
            }
            
            // At the lowest brightness setting, switch to night light mode.
            // In the Yeelight integration in Home Assistant, this feature is
            // exposed trough a separate switch. I have found that the switch
            // is both confusing and made me run into issues when automating
            // the lights.
            // I don't simply check for a brightness at or below 0.01 (1%),
            // because the lowest brightness setting from Home Assistant
            // turns up as 0.011765 in here (which is 3/255).
            if (values.get_brightness() < 0.012f) {
                // TODO make the color implementations return a DutyCycles object?
                // TODO Use polymorphic color classes?
                red = night_light_.red;
                green = night_light_.green;
                blue = night_light_.blue;
                white = night_light_.white;
                return;
            }

            // White light mode: temperature + brightness.
            if (values.get_white() > 0.0f) {
                auto temperature = values.get_color_temperature();
                white_light_.set_color(temperature, values.get_brightness());
                red = white_light_.red;
                green = white_light_.green;
                blue = white_light_.blue;
                white = white_light_.white;
                return;
            }

            // RGB color mode: red, green, blue + brightness.
            rgb_light_.set_color(
                values.get_red(), values.get_green(), values.get_blue(),
                values.get_brightness());
            red = rgb_light_.red;
            green = rgb_light_.green;
            blue = rgb_light_.blue;
            white = rgb_light_.white;
        }

    protected:
        ColorWhiteLight white_light_;
        ColorRGBLight rgb_light_;
        ColorNightLight night_light_;
    };

    /// This class is used to handle color transition requirements.
    ///
    /// When using the default ESPHome logic, transitioning is done by
    /// transitioning all light properties linearly from the original
    /// values to the new values, and letting the light output object
    /// translate these properties into light outputs on every step of the
    /// way. While this does work, it does not work nicely.
    ///
    /// For example, when transitioning from warm to cold white light,
    /// the color temperature would be transitioned from the old value to
    /// the new value. While doing so, the transition hits the middle
    /// white light setting, which shows up as a bright flash in the
    /// middle of the transition. The original firmware however, shows a
    /// smooth transition from warm to cold white light, without any flash.
    ///
    /// This class handles transitions by not varying the light properties
    /// over time, but by transitioning the LEDC duty cycle output levels
    /// over time. This matches the behavior of the original firmware.
    class TransitionHandler {
    public:
        TransitionHandler(LightStateDataExposer *exposer) : exposer_(exposer) {}

        bool handle() {
            if (!do_handle_()) {
                active_ = false;
                return false;
            }

            if (is_fresh_transition_()) {
               auto start = exposer_->get_transformer_values();
               auto end = exposer_->get_transformer_end_values();
               active_ = true;
            }

            return true;
        }

    protected:
        LightStateDataExposer *exposer_;
        bool active_ = false;
        DutyCycles start_;
        DutyCycles end_;

        /// Checks if this class will handle the light output logic.
        /// This is the case when a transformer is active and this
        /// transformer does implement a transitioning effect.
        bool do_handle_() {
            if (!exposer_->has_active_transformer())
                return false;
            if (!exposer_->transformer_is_transition())
                return false;
            return true;
        }

        /// Checks if a fresh transitioning is started.
        /// A transitioning is fresh when either no transition is known to
        /// be in progress or when a new end state is found during an
        /// ongoing transition.
        bool is_fresh_transition_() {
            bool is_fresh = false;
            if (active_ == false) {
                is_fresh = true;
            }
            return is_fresh;
        }
    };

    /// An implementation of the LightOutput interface for the Yeelight
    /// Bedside Lamp 2. The function of this class is to translate a
    /// required light state into actual physicial GPIO output signals
    /// to drive the device's LED circuitry.
    class YeelightBS2LightOutput : public Component, public light::LightOutput {
    public:
        /// Set the LEDC output for the red LED circuitry channel.
        void set_red_output(ledc::LEDCOutput *red) {
            red_ = red;
        }

        /// Set the LEDC output for the green LED circuitry channel.
        void set_green_output(ledc::LEDCOutput *green) {
            green_ = green;
        }

        /// Set the LEDC output for the blue LED circuitry channel.
        void set_blue_output(ledc::LEDCOutput *blue) {
            blue_ = blue;
        }

        /// Set the LEDC output for the white LED circuitry channel.
        void set_white_output(ledc::LEDCOutput *white) {
            white_ = white;
        }

        /// Set the first GPIO binary output, used as internal master
        /// switch for the LED light circuitry.
        void set_master1_output(gpio::GPIOBinaryOutput *master1) {
            master1_ = master1;
        }

        /// Set the second GPIO binary output, used as internal master
        /// switch for the LED light circuitry.
        void set_master2_output(gpio::GPIOBinaryOutput *master2) {
            master2_ = master2;
        }

        /// Returns a LightTraits object, which is used to explain to the
        /// outside world (e.g. Home Assistant) what features are supported
        /// by this device.
        light::LightTraits get_traits() override
        {
            auto traits = light::LightTraits();
            traits.set_supports_rgb(true);
            traits.set_supports_color_temperature(true);
            traits.set_supports_brightness(true);
            traits.set_supports_rgb_white_value(false);
            traits.set_supports_color_interlock(true);
            traits.set_min_mireds(MIRED_MIN);
            traits.set_max_mireds(MIRED_MAX);
            return traits;
        }

        /// Tranlates a requested light state into physicial GPIO outputs.
        void write_state(light::LightState *state)
        {
            auto values = state->current_values;

            // Power down the light when its state is 'off'.
            if (values.get_state() == 0)
            {
                red_->set_level(1.0f);
                green_->set_level(1.0f);
                blue_->set_level(1.0f);
                white_->set_level(0.0f);
                master2_->turn_off();
                master1_->turn_off();
#ifdef TRANSITION_TO_OFF_BUGFIX
                previous_state_ = -1;
                previous_brightness_ = 0;
#endif
                return;
            }

            if (transition_->handle()) {
                ESP_LOGD(TAG, "HANDLE transition!");
            }

#ifdef TRANSITION_TO_OFF_BUGFIX
            // Remember the brightness that is used when the light is fully ON.
            auto brightness = values.get_brightness();
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

            duty_cycles_.set_light_color_values(values);
            master2_->turn_on();
            master1_->turn_on();
            red_->set_level(duty_cycles_.red);
            green_->set_level(duty_cycles_.green);
            blue_->set_level(duty_cycles_.blue);
            white_->set_level(duty_cycles_.white);
        }

    protected:
        ledc::LEDCOutput *red_;
        ledc::LEDCOutput *green_;
        ledc::LEDCOutput *blue_;
        ledc::LEDCOutput *white_;
        esphome::gpio::GPIOBinaryOutput *master1_;
        esphome::gpio::GPIOBinaryOutput *master2_;
        TransitionHandler *transition_;
        ColorTranslator duty_cycles_; // TODO move to own class DefaultHandler
#ifdef TRANSITION_TO_OFF_BUGFIX
        float previous_state_ = 1;
        float previous_brightness_ = -1;
#endif

        friend class YeelightBS2LightState;

        /// Called by the YeelightBS2LightState class, to set the object that
        /// can be used to access protected data from the light state object.
        void set_light_state_data_exposer(LightStateDataExposer *exposer) {
            transition_ = new TransitionHandler(exposer);
        }
    };

    class YeelightBS2LightState : public light::LightState, public LightStateDataExposer
    {
    public:
        YeelightBS2LightState(const std::string &name, YeelightBS2LightOutput *output) : light::LightState(name, output) {
            output->set_light_state_data_exposer(this);
        }

        bool has_active_transformer() {
            return this->transformer_ != nullptr;
        }

        bool transformer_is_transition() {
            return this->transformer_->is_transition();
        }

        light::LightColorValues get_transformer_values() {
            return this->transformer_->get_values();
        }

        light::LightColorValues get_transformer_end_values() {
            return this->transformer_->get_end_values();
        }

        float get_transformer_progress() {
            return this->transformer_->get_progress();
        }
    };
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
