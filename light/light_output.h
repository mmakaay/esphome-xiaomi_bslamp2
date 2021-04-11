#pragma once 

#include "../common.h"
#include "../yeelight_bs2_hub.h"
#include "color_instant_handler.h"
#include "color_transition_handler.h"
#include "esphome/components/ledc/ledc_output.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * A LightOutput class for the Yeelight Bedside Lamp 2.
 *
 * The function of this class is to translate a required light state
 * into actual physicial GPIO output signals to drive the device's LED
 * circuitry. It forms the glue between the physical device and the
 * logical light color input.
 */
class YeelightBS2LightOutput : public Component, public light::LightOutput {
public:
    /** Sets the Yeelight BS2 hub component. */ 
    void set_hub(YeelightBS2Hub *hub) { hub_ = hub; }

    /**
     * Returns a LightTraits object, which is used to explain to the outside
     * world (e.g. Home Assistant) what features are supported by this device.
     */
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

    void add_on_state_callback(std::function<void(light::LightColorValues)> &&callback) {
          this->state_callback_.add(std::move(callback));
    }

    /**
     * Applies a requested light state to the physicial GPIO outputs.
     */
    void write_state(light::LightState *state)
    {
        auto values = state->current_values;

        // The color must either be set instantly, or the color is
        // transitioning to an end color. The transition handler will do its
        // own inspection to see if a transition is currently active or not.
        // Based on the outcome, use either the instant or transition handler.
        GPIOOutputs *delegate;
        if (transition_handler_->set_light_color_values(values)) {
            delegate = transition_handler_;
        } else {
            instant_handler_->set_light_color_values(values);
            delegate = instant_handler_;
        }

        // Note: one might think that it is more logical to turn on the LED
        // circuitry master switch after setting the individual channels,
        // but this is the order that was used by the original firmware. I
        // tried to stay as close as possible to the original behavior, so
        // that's why these GPIOs are turned on at this point.
        if (values.get_state() != 0)
        {
            hub_->master2->turn_on();
            hub_->master1->turn_on();
        }

        // Apply the current GPIO output levels from the selected handler.
        hub_->red->set_level(delegate->red);
        hub_->green->set_level(delegate->green);
        hub_->blue->set_level(delegate->blue);
        hub_->white->set_level(delegate->white);

        if (values.get_state() == 0)
        {
            hub_->master2->turn_off();
            hub_->master1->turn_off();
        }

        this->state_callback_.call(values);
    }

protected:
    YeelightBS2Hub *hub_;
    GPIOOutputs *transition_handler_;
    GPIOOutputs *instant_handler_ = new ColorInstantHandler();
    CallbackManager<void(light::LightColorValues)> state_callback_{};

    friend class YeelightBS2LightState;

    /**
     * Called by the YeelightBS2LightState class, to set the object that can be
     * used to access the protected LightTransformer data from the LightState
     * object.
     */
    void set_transformer_inspector(LightStateTransformerInspector *exposer) {
        transition_handler_ = new ColorTransitionHandler(exposer);
    }
};

/**
 * This custom LightState class is used to provide access to the protected
 * LightTranformer information in the LightState class.
 *
 * This class is used by the ColorTransitionHandler class to inspect if
 * an ongoing light color transition is active in a LightState object.
 */
class YeelightBS2LightState : public light::LightState, public LightStateTransformerInspector
{
public:
    YeelightBS2LightState(const std::string &name, YeelightBS2LightOutput *output) : light::LightState(name, output) {
        output->set_transformer_inspector(this);
    }

    bool is_active() { return this->transformer_ != nullptr; }
    bool is_transition() { return this->transformer_->is_transition(); }
    light::LightColorValues get_end_values() { return this->transformer_->get_end_values(); }
    float get_progress() { return this->transformer_->get_progress(); }
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome