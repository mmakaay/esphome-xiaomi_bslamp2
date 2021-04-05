#pragma once

#include <array>
#include <stdexcept>

namespace esphome {
namespace yeelight {
namespace bs2 {

class ColorNightLight
{
public:
    // Based on measurements using the original device firmware.
    float red   = 0.968f;
    float green = 0.968f;
    float blue  = 0.972f;
    float white = 0.0f;

    void set_color(float red, float green, float blue, float brightness, float state)
    {
    }
};

} // namespace yeelight_bs2
} // namespace yeelight
} // namespace bs2
