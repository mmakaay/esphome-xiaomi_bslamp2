#pragma once 

#include "../common.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This custom LightState class is used to provide access to the protected
 * LightTranformer information in the LightState class.
 *
 * This class is used by the ColorTransitionHandler class to inspect if
 * an ongoing light color transition is active in the LightState object.
 */
class XiaomiBslamp2LightState : public light::LightState, public LightStateTransformerInspector
{
public:
    XiaomiBslamp2LightState(const std::string &name, XiaomiBslamp2LightOutput *output) : light::LightState(name, output) {
        output->set_transformer_inspector(this);
    }

    bool is_active() { return transformer_ != nullptr; }
    bool is_transition() { return transformer_->is_transition(); }
    light::LightColorValues get_end_values() { return transformer_->get_end_values(); }
    float get_progress() { return transformer_->get_progress(); }
};
    
} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
