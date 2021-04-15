#pragma once 

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

// Light modes that can be reported by implementations of GPIOOutputs.
static const std::string LIGHT_MODE_UNKNOWN { "unknown" };
static const std::string LIGHT_MODE_OFF { "off" };
static const std::string LIGHT_MODE_RGB { "rgb" };
static const std::string LIGHT_MODE_WHITE { "white" };
static const std::string LIGHT_MODE_NIGHT { "night" };

/**
 * This abstract class is used for implementing classes that translate
 * LightColorValues into the required GPIO PWM duty cycle levels to represent
 * the requested color on the physical device.
 */
class GPIOOutputs {
public:
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float white = 0.0f;
    std::string light_mode = LIGHT_MODE_OFF;

    /**
     * Sets the red, green, blue, white fields to the PWM duty cycles
     * that are required to represent the requested light color for
     * the provided LightColorValues input.
     *
     * Returns true when the input can be handled, false otherwise.
     */
    virtual bool set_light_color_values(light::LightColorValues v) = 0;

    /**
     * Copies the current output values to another GPIOOutputs object.
     */
    void copy_to(GPIOOutputs *other) {
        other->red = red;
        other->green = green;
        other->blue = blue;
        other->white = white;
        other->light_mode = light_mode;
    }

    void log(const char *prefix) {
        ESP_LOGD(TAG, "%s: RGB=[%f,%f,%f], white=%f",
            prefix, red, green, blue, white);
    }
};
    
} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
