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

    /// This struct is used to hold GPIO pin duty cycles.
    struct DutyCycles {
        float red;
        float green;
        float blue;
        float white;
    };
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
