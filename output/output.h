#pragma once 

#include <cmath>
#include "../common.h"
#include "../front_panel_hal.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

/**
 * An output, used for controlling the front panel light on the 
 * Yeelight Bedside Lamp 2 front panel.
 */
class YeelightBS2FrontPanelLight : public output::FloatOutput, public Component {
public:
    void set_parent(FrontPanelHAL *front_panel) { front_panel_ = front_panel; }

    void write_state(float level) {
        front_panel_->set_light_level(level);
    }

protected:
    FrontPanelHAL *front_panel_;
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
