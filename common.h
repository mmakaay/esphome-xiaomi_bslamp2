#pragma once 

namespace esphome {
namespace yeelight {
namespace bs2 {

    /// A tag, used for logging.
    static const char *TAG = "yeelight_bs2";

    /// The minimum color temperature in mired. Same as supported by
    /// the original Yeelight firmware.
    static const int MIRED_MIN = 153;

    /// The maximum color temperature in mired. Same as supported by
    /// the original Yeelight firmware.
    static const int MIRED_MAX = 588;

    /// This abstract class is used for building classes that translate
    /// LightColorValues into the required GPIO pin outputs to represent
    /// the requested color on the device.
    class GPIOOutputs {
    public:
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float white = 0.0f;
        light::LightColorValues values;

        /// Set the red, green, blue, white fields to the PWM duty cycles
        /// that are required to represent the requested light color for
        /// the provided LightColorValues input.
        /// Returns true when the class can handle the input, false otherwise.
        virtual bool set_light_color_values(light::LightColorValues v) = 0;

        /// Copy the output values to another GPIOOutputs object.
        void copy_to(GPIOOutputs *other) {
            other->red = red;
            other->green = green;
            other->blue = blue;
            other->white = white;
            other->values = values;
        }

        void log(const char *prefix) {
            ESP_LOGD(TAG, "%s: RGB=[%f,%f,%f], white=%f",
                prefix, red, green, blue, white);
        }
    };
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
