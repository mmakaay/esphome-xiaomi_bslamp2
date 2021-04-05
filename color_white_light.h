/**
 * This code implements the white light mode (based on temperature +
 * brigtness) for the Yeelight Bedside Lamp 2.
 */
#pragma once

#include <array>
#include <stdexcept>

namespace esphome {
namespace yeelight {
namespace bs2 {

// Same range as supported by the original Yeelight firmware.
static const int MIRED_MAX = 153;
static const int MIRED_MIN = 588;

struct RGBWLevelsByTemperature {
    float from_temperature;
    float red;
    float green;
    float blue;
    float white;
};

using RGBWLevelsTable = std::array<RGBWLevelsByTemperature, 15>;

static const RGBWLevelsTable rgbw_levels_1_ {{
    { 501.0f, 0.873f, 0.907f, 1.000f,  0.063f },
    { 455.0f, 0.873f, 0.896f, 1.000f,  0.063f },
    { 417.0f, 0.873f, 0.891f, 1.000f,  0.068f },
    { 371.0f, 0.873f, 0.880f, 1.000f,  0.070f },
    { 334.0f, 0.873f, 0.887f, 1.000f,  0.088f },
    { 313.0f, 0.882f, 0.904f, 1.000f,  0.128f },
    { 295.0f, 0.947f, 1.000f, 0.968f,  0.145f },
    { 251.0f, 0.999f, 1.000f, 1.000f,  0.155f },
    { 223.0f, 1.000f, 0.899f, 0.921f,  0.130f },
    { 201.0f, 1.000f, 0.873f, 0.908f,  0.115f },
    { 182.0f, 1.000f, 0.873f, 0.901f,  0.103f },
    { 173.0f, 1.000f, 0.873f, 0.904f,  0.094f },
    { 167.0f, 1.000f, 0.873f, 0.891f,  0.098f },
    { 154.0f, 1.000f, 0.873f, 0.894f,  0.090f },
    { 153.0f, 1.000f, 0.873f, 0.892f,  0.088f }
}};

static const RGBWLevelsTable rgbw_levels_100_ {{
    { 501.0f, 0.000f, 0.344f, 1.000f, 0.068f },
    { 455.0f, 0.000f, 0.237f, 1.000f, 0.093f },
    { 417.0f, 0.000f, 0.186f, 1.000f, 0.120f },
    { 371.0f, 0.000f, 0.149f, 1.000f, 0.167f },
    { 334.0f, 0.000f, 0.135f, 1.000f, 0.325f },
    { 313.0f, 0.097f, 0.314f, 1.000f, 0.740f },
    { 295.0f, 0.745f, 1.000f, 0.953f, 0.905f },
    { 251.0f, 1.000f, 1.000f, 1.000f, 1.000f },
    { 223.0f, 1.000f, 0.267f, 0.485f, 0.765f },
    { 201.0f, 1.000f, 0.000f, 0.355f, 0.609f },
    { 182.0f, 1.000f, 0.000f, 0.282f, 0.489f },
    { 173.0f, 1.000f, 0.000f, 0.313f, 0.392f },
    { 167.0f, 1.000f, 0.000f, 0.180f, 0.422f },
    { 154.0f, 1.000f, 0.000f, 0.218f, 0.368f },
    { 153.0f, 1.000f, 0.000f, 0.187f, 0.335f }
}};

class ColorWhiteLight
{
public:
    float red = 0;
    float green = 0;
    float blue = 0;
    float white = 0;

    void set_color(float temperature, float brightness)
    {
        temperature = clamp_temperature_(temperature);
        brightness = clamp_brightness_(brightness);

        auto levels_1 = lookup_in_table_(rgbw_levels_1_, temperature);
        auto levels_100 = lookup_in_table_(rgbw_levels_100_, temperature);

        red = interpolate_(levels_1.red, levels_100.red, brightness);
        green = interpolate_(levels_1.green, levels_100.green, brightness);
        blue = interpolate_(levels_1.blue, levels_100.blue, brightness);
        white = interpolate_(levels_1.white, levels_100.white, brightness);
    }

protected:
    float clamp_temperature_(float temperature)
    {
        if (temperature < MIRED_MAX)
            temperature = MIRED_MAX;
        else if (temperature > MIRED_MIN) 
            temperature = MIRED_MIN;
        return temperature;
    }

    float clamp_brightness_(float brightness)
    {
        if (brightness < 0.01f)
            brightness = 0.01f;
        else if (brightness > 1.00f)
            brightness = 1.00f;
        return brightness;
    }

    RGBWLevelsByTemperature lookup_in_table_(RGBWLevelsTable table, float temperature)
    {
        for (RGBWLevelsByTemperature& item : table) 
            if (temperature >= item.from_temperature) 
                return item;
        throw std::invalid_argument("received too low temperature");
    }

    float interpolate_(float level_1, float level_100, float brightness)
    {
        auto coefficient = (level_100 - level_1) / 0.99f;
        auto level = level_1 + (brightness - 0.01f) * coefficient;
        return level;
    }
};

} // namespace bs2
} // namespace yeelight
} // namespace esphome
