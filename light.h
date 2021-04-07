#pragma once 

namespace esphome {
namespace yeelight {
namespace bs2 {

    /// This is an interface definition that is used to extend the
    /// YeelightBS2LightOutput class with methods to access properties
    /// of an active LightTranformer from the TransitionHandler class.
    ///
    /// The transformer is protected in the light output class, making
    /// it impossible to access these properties directly from the
    /// light output class.
    class LightStateTransformerInspector {
    public:
        virtual bool is_active() = 0;
        virtual bool is_transition() = 0;
        virtual light::LightColorValues get_end_values() = 0;
        virtual float get_progress() = 0;
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
    class TransitionHandler : public GPIOOutputs {
    public:
        TransitionHandler(LightStateTransformerInspector *inspector) : transformer_(inspector) {}

        bool set_light_color_values(light::LightColorValues values) {
            if (!light_state_has_active_transition_()) {
                // Remember the last active light color values. When a transition
                // is detected, use these as the starting point. It is not possible
                // to use the current values at that point, because the transition
                // is already in progress by the time the transition is detected.
                start_values = values;

                active_ = false;
                return false;
            }

            // When a fresh transition is started, then compute the GPIO outputs
            // to use for both the start and end point. This transition handler
            // will then transition linearly between these two.
            if (is_fresh_transition_()) {
                start_->set_light_color_values(start_values);
                end_->set_light_color_values(transformer_->get_end_values());
                active_ = true;
            }
            // When a transition is modified, then use the current GPIO outputs
            // as the new starting point.
            else if (is_modified_transition_()) {
                this->copy_to(start_);
                end_->set_light_color_values(transformer_->get_end_values());
            }

            // Determine the required GPIO outputs for the current transition progress.
            progress_ = transformer_->get_progress();
            auto smoothed = light::LightTransitionTransformer::smoothed_progress(progress_);
            red = esphome::lerp(smoothed, start_->red, end_->red);
            green = esphome::lerp(smoothed, start_->green, end_->green);
            blue = esphome::lerp(smoothed, start_->blue, end_->blue);
            white = esphome::lerp(smoothed, start_->white, end_->white);

            return true;
        }

    protected:
        bool active_ = false;
        float progress_ = 0.0f;
        LightStateTransformerInspector *transformer_;
        light::LightColorValues start_values;
        GPIOOutputs *start_ = new ColorTranslator();
        GPIOOutputs *end_ = new ColorTranslator();

        /// Checks if the LightState object currently has an active LightTransformer.
        bool light_state_has_active_transition_() {
            if (!transformer_->is_active())
                return false;
            if (!transformer_->is_transition())
                return false;
            return true;
        }

        /// Checks if a fresh transitioning is started.
        /// A transitioning is fresh when no existing transition is active.
        bool is_fresh_transition_() {
            return active_ == false;
        }

        /// Checks if a new end state is set, while an existing transition
        /// is active. This might be detected in two ways:
        /// - the end color has been updated
        /// - the progress has been reverted
        bool is_modified_transition_() {
            auto new_end_values = transformer_->get_end_values();
            auto new_progress = transformer_->get_progress();
            return new_end_values != end_->values || new_progress < progress_;
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

        /// Applies a requested light state to the physicial GPIO outputs.
        void write_state(light::LightState *state)
        {
            auto values = state->current_values;

            // Turn off the light when its state is 'off'.
            if (values.get_state() == 0)
            {
                ESP_LOGD(TAG, "Turn off the light");
                red_->set_level(1.0f);
                green_->set_level(1.0f);
                blue_->set_level(1.0f);
                white_->set_level(0.0f);
                master2_->turn_off();
                master1_->turn_off();
                return;
            }

            GPIOOutputs *delegate;
            if (transition_handler_->set_light_color_values(values)) {
                transition_handler_->log("TRANSITION");
                delegate = transition_handler_;
            } else {
                instant_handler_->set_light_color_values(values);
                instant_handler_->log("INSTANT");
                delegate = instant_handler_;
            }

            delegate->set_light_color_values(values);
            master2_->turn_on();
            master1_->turn_on();
            red_->set_level(delegate->red);
            green_->set_level(delegate->green);
            blue_->set_level(delegate->blue);
            white_->set_level(delegate->white);
        }

    protected:
        ledc::LEDCOutput *red_;
        ledc::LEDCOutput *green_;
        ledc::LEDCOutput *blue_;
        ledc::LEDCOutput *white_;
        esphome::gpio::GPIOBinaryOutput *master1_;
        esphome::gpio::GPIOBinaryOutput *master2_;
        GPIOOutputs *transition_handler_;
        GPIOOutputs *instant_handler_ = new ColorTranslator();

        friend class YeelightBS2LightState;

        /// Called by the YeelightBS2LightState class, to set the object that
        /// can be used to access protected data from the light state object.
        void set_transformer_inspector(LightStateTransformerInspector *exposer) {
            transition_handler_ = new TransitionHandler(exposer);
        }
    };

    class YeelightBS2LightState : public light::LightState, public LightStateTransformerInspector
    {
    public:
        YeelightBS2LightState(const std::string &name, YeelightBS2LightOutput *output) : light::LightState(name, output) {
            output->set_transformer_inspector(this);
        }

        bool is_active() {
            return this->transformer_ != nullptr;
        }

        bool is_transition() {
            return this->transformer_->is_transition();
        }

        light::LightColorValues get_end_values() {
            return this->transformer_->get_end_values();
        }

        float get_progress() {
            return this->transformer_->get_progress();
        }
    };
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
