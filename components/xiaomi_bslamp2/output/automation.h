#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "output.h"
#include <cmath>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

template<typename... Ts> class SetLEDsAction : public Action<Ts...> {
 public:
  explicit SetLEDsAction(XiaomiBslamp2FrontPanelOutput *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(uint16_t, leds)

  void play(Ts... x) override {
    uint16_t value = this->leds_.value(x...);
    parent_->set_leds(value);
  }

 protected:
  XiaomiBslamp2FrontPanelOutput *parent_;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
