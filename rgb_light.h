#pragma once

#include <array>
#include <stdexcept>

namespace esphome {
namespace rgbww {
namespace yeelight_bs2 {

class RGBLight
{
public:
    float red = 0;
    float green = 0;
    float blue = 0;
    float white = 0;

    void set_color(float red, float green, float blue, float brightness, float state)
    {
        // Overall, the RGB colors are very usable when simply scaling the
        // RGB channels with the brightness, but around the white point,
        // the color is a bit on the red side of the spectrum. The following
        // scaling was created to fix that.
        // These functions were created, based on actual measurements while
        // using the original firmware.
        auto b = brightness * 100.0f;
        auto red_w = 1.00f - (-0.0000121426 * b * b - 0.147576 * b + 93.2335) / 100.0f;
        auto green_w = 1.00f - (-0.0000242425 * b * b - 0.340449 * b + 88.4423) / 100.0f;
        auto blue_w = 1.00f - (-0.0000085869 * b * b - 0.109649 * b + 94.2026) / 100.0f;

        // For colors that are not around the white point, we can scale the
        // RGB channels with the requested brightness, resulting in a very
        // usable color. Not 100% the same as the original firmware, but
        // sometimes even better IMO.
        auto red_c = red * brightness;
        auto green_c = green * brightness;
        auto blue_c = blue * brightness;

        // The actual RGB values are a weighed mix of the above two.
        // The closer to the white point, the more the white point
        // value applies.
        auto level_red = (red_w * ((green+blue)/2)) + (red_c * (1-(green+blue)/2));
        auto level_green = (green_w * ((red+blue)/2)) + (green_c * (1-(red+blue)/2));
        auto level_blue = (blue_w * ((red+green)/2)) + (blue_c * (1-(red+green)/2));
        if (red == 1 && green == 1 && blue == 1) {
            level_red = red_w;
            level_green = green_w;
            level_blue = blue_w;
        }

        // Invert the signal. The LEDs in the lamp's circuit are brighter
        // when the pwm levels on the GPIO pins are lower.
        this->red = 1.0f - level_red;
        this->green = 1.0f - level_green;
        this->blue = 1.0f - level_blue;
    }
};

} // namespace yeelight_bs2
} // namespace rgbww
} // namespace esphome
