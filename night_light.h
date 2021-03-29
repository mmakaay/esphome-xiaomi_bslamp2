#pragma once

#include <array>
#include <stdexcept>

namespace esphome {
namespace rgbww {
namespace yeelight_bs2 {

class NightLight
{
public:
    // Based on measurements using the original device firmware.
    //float red   = 0.968f;
    //float green = 0.960f;
    //float blue  = 0.978f;
    float red   = 0.968f;
    float green = 0.968f;
    float blue  = 0.972f;
    float white = 0.0f;

    void set_color(float red, float green, float blue, float brightness, float state)
    {
    }
};

} // namespace yeelight_bs2
} // namespace rgbww
} // namespace esphome
