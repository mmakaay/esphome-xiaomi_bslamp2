#pragma once

#include <array>
#include <stdexcept>

namespace esphome {
namespace rgbww {
namespace yeelight_bs2 {

// Same range as supported by the original Yeelight firmware.
static const int MIRED_MAX = 153;
static const int MIRED_MIN = 588;

struct RgbwLevelsByTemperature {
    float from_temperature;
    float red;
    float green;
    float blue;
    float white;
};

using RgbwLevelsTable = std::array<RgbwLevelsByTemperature, 15>;

static const RgbwLevelsTable rgbw_levels_1_ {{
    { 501.0f,  87.3f,  90.7f, 100.0f,   6.3f },
    { 455.0f,  87.3f,  89.6f, 100.0f,   6.3f },
    { 417.0f,  87.3f,  89.1f, 100.0f,   6.8f },
    { 371.0f,  87.3f,  88.0f, 100.0f,   7.0f },
    { 334.0f,  87.3f,  88.7f, 100.0f,   8.8f },
    { 313.0f,  88.2f,  90.4f, 100.0f,  12.8f },
    { 295.0f,  94.7f, 100.0f,  96.8f,  14.5f },
    { 251.0f,  99.9f, 100.0f, 100.0f,  15.5f },
    { 223.0f, 100.0f,  89.9f,  92.1f,  13.0f },
    { 201.0f, 100.0f,  87.3f,  90.8f,  11.5f },
    { 182.0f, 100.0f,  87.3f,  90.1f,  10.3f },
    { 173.0f, 100.0f,  87.3f,  90.4f,   9.4f },
    { 167.0f, 100.0f,  87.3f,  89.1f,   9.8f },
    { 154.0f, 100.0f,  87.3f,  89.4f,   9.0f },
    { 153.0f, 100.0f,  87.3f,  89.2f,   8.8f }
}};

static const RgbwLevelsTable rgbw_levels_100_ {{
    { 501.0f,   0.0f,  34.4f, 100.0f,   6.8f },
    { 455.0f,   0.0f,  23.7f, 100.0f,   9.3f },
    { 417.0f,   0.0f,  18.6f, 100.0f,  12.0f },
    { 371.0f,   0.0f,  14.9f, 100.0f,  16.7f },
    { 334.0f,   0.0f,  13.5f, 100.0f,  32.5f },
    { 313.0f,   9.7f,  31.4f, 100.0f,  74.0f },
    { 295.0f,  74.5f, 100.0f,  95.3f,  90.5f },
    { 251.0f, 100.0f, 100.0f, 100.0f, 100.0f },
    { 223.0f, 100.0f,  26.7f,  48.5f,  76.5f },
    { 201.0f, 100.0f,   0.0f,  35.5f,  60.9f },
    { 182.0f, 100.0f,   0.0f,  28.2f,  48.9f },
    { 173.0f, 100.0f,   0.0f,  31.3f,  39.2f },
    { 167.0f, 100.0f,   0.0f,  18.0f,  42.2f },
    { 154.0f, 100.0f,   0.0f,  21.8f,  36.8f },
    { 153.0f, 100.0f,   0.0f,  18.7f,  33.5f }
}};

class WhiteLight
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

    RgbwLevelsByTemperature lookup_in_table_(RgbwLevelsTable table, float temperature)
    {
        for (RgbwLevelsByTemperature& item : table) 
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

} // namespace yeelight_bs2
} // namespace rgbww
} // namespace esphome
