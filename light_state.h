#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/light_output.h" 


namespace esphome {
namespace yeelight {
namespace bs2 {

    class YeelightBS2LightState : public light::LightState
    {
        public:
            YeelightBS2LightState(const std::string &name, light::LightOutput *output) : light::LightState(name, output) {}
    };

} // namespace bs2
} // namespace yeelight
} // namespace esphome
