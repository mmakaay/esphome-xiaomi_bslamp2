#pragma once

#include "../common.h"
#include "gpio_outputs.h"
#include "color_instant_handler.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This is an interface definition that is used to extend the LightState
 * class with functionality to inspect LightTransformer data from
 * within other classes.
 *
 * This interface is required for the ColorTransitionHandler, so it can
 * check whether or not a light color transition is in progress.
 */
class LightStateTransformerInspector {
public:
    virtual bool is_active() = 0;
    virtual bool is_transition() = 0;
    virtual light::LightColorValues get_end_values() = 0;
    virtual float get_progress() = 0;
};

/**
 * This class is used to handle specific light color transition requirements
 * for the device.
 * 
 * When using the default ESPHome logic, transitioning is done by
 * transitioning all light properties linearly from the original values to
 * the new values, and letting the light output object translate these
 * properties into light outputs on every step of the way. While this does
 * work, it does not work nicely.
 * 
 * For example, when transitioning from warm to cold white light, the color
 * temperature would be transitioned from the old value to the new value.
 * While doing so, the transition hits the middle white light setting, which
 * shows up as a bright flash in the middle of the transition. The original
 * firmware however, shows a smooth transition from warm to cold white
 * light, without any flash.
 * 
 * This class handles transitions by not varying the light properties over
 * time, but by transitioning the LEDC duty cycle output levels over time.
 * This matches the behavior of the original firmware.
 */
class ColorTransitionHandler : public GPIOOutputs {
public:
    ColorTransitionHandler(LightStateTransformerInspector *inspector) : transformer_(inspector) {}

    light::LightColorValues get_end_values() {
        return end_light_values_;
    }

    bool set_light_color_values(light::LightColorValues values) {
        if (!light_state_has_active_transition_()) {
            // Remember the last active light color values. When a transition
            // is detected, we'll use these as the starting point. It is not
            // possible to use the current values at that point, because the
            // transition is already in progress by the time the transition
            // is detected.
            start_light_values_ = values;

            active_ = false;
            return false;
        }

        // When a fresh transition is started, then compute the GPIO outputs
        // to use for both the start and end point. This transition handler
        // will then transition linearly between these two.
        if (is_fresh_transition_()) {
            start_->set_light_color_values(start_light_values_);
            end_light_values_ = transformer_->get_end_values();
            end_->set_light_color_values(end_light_values_);
            active_ = true;
        }
        // When a transition is modified, then use the current GPIO outputs
        // as the new starting point.
        else if (is_modified_transition_()) {
            this->copy_to(start_);
            end_light_values_ = transformer_->get_end_values();
            end_->set_light_color_values(end_light_values_);
        }

        light_mode = end_->light_mode;
        progress_ = transformer_->get_progress();

        // Determine required GPIO outputs for current transition progress.

        // In night light mode, do not use actual transitions. Transitioning
        // between colors at the very low LED output levels of the night light,
        // results in light drops, which are plain ugly to watch. 
        if (light_mode == "night") {
            red = end_->red;
            green = end_->green;
            blue = end_->blue;
            white = end_->white;
        }
        // In other light modes, apply smooth transitioning.
        else {
            auto smoothed = light::LightTransitionTransformer::smoothed_progress(progress_);
            red = esphome::lerp(smoothed, start_->red, end_->red);
            green = esphome::lerp(smoothed, start_->green, end_->green);
            blue = esphome::lerp(smoothed, start_->blue, end_->blue);
            white = esphome::lerp(smoothed, start_->white, end_->white);
        }

        return true;
    }

protected:
    bool active_ = false;
    float progress_ = 0.0f;
    LightStateTransformerInspector *transformer_;
    light::LightColorValues start_light_values_;
    light::LightColorValues end_light_values_;
    GPIOOutputs *start_ = new ColorInstantHandler();
    GPIOOutputs *end_ = new ColorInstantHandler();

    /**
     * Checks if the LightState currently has an active LightTransformer.
     */
    bool light_state_has_active_transition_() {
        if (!transformer_->is_active())
            return false;
        if (!transformer_->is_transition())
            return false;
        return true;
    }

    /**
     * Checks if a fresh transitioning is started.
     * A transitioning is fresh when no existing transition is active.
     */
    bool is_fresh_transition_() {
        return active_ == false;
    }

    /**
     * Checks if a new end state is set, while an existing transition
     * is active. This might be detected in two ways:
     * - the end color has been updated
     * - the progress has been reverted
     */
    bool is_modified_transition_() {
        auto new_end_light_values = transformer_->get_end_values();
        auto new_progress = transformer_->get_progress();
        return (
            new_end_light_values != end_light_values_ ||
            new_progress < progress_
        );
    }
};

} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
