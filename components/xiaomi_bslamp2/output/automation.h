#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "../front_panel_hal.h"
#include "output.h"
#include <cmath>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

template<typename... Ts> class SetLEDsAction : public Action<Ts...> {
 public:
  explicit SetLEDsAction(XiaomiBslamp2FrontPanelOutput *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(int, mode)
  TEMPLATABLE_VALUE(uint16_t, leds)

  void play(Ts... x) override {
    uint16_t mode = this->mode_.value(x...);
    uint16_t value = this->leds_.value(x...);
    switch (mode) {
      case 0:
        parent_->turn_off_leds(value);
        break;
      case 1:
        parent_->turn_on_leds(value);
        break;
      case 2:
        parent_->set_leds(value);
        break;
    }
  }

 protected:
  XiaomiBslamp2FrontPanelOutput *parent_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
