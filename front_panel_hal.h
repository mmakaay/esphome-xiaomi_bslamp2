#pragma once 

#include "common.h"
#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

class FrontPanelHAL : public Component, public i2c::I2CDevice {
public:
    void set_trigger_pin(GPIOPin *pin) { i2c_trigger_pin_ = pin; }

    void setup() {
        ESP_LOGCONFIG(TAG, "Setting up I2C trigger pin interrupt...");
        this->i2c_trigger_pin_->setup();
        this->i2c_trigger_pin_->attach_interrupt(
            FrontPanelHAL::isr, this, FALLING);
    }

    void dump_config() {
        ESP_LOGCONFIG(TAG, "I2C");
        LOG_PIN("  Interrupt pin: ", this->i2c_trigger_pin_);
    }

    void loop() {
        if (this->queue_length > 0) {
            this->queue_length--;
            ESP_LOGD(TAG, "EVENT!");
        }
    }

protected:
    // The GPIO pin that is used by the front panel to notify the ESP that
    // a touch/release event can be read using I2C.
    GPIOPin *i2c_trigger_pin_;

    // The ISR that is used for handling event interrupts.
    static void isr(FrontPanelHAL *store);

    // The number of unhandled event interrupts.
    volatile int queue_length = 0;
};

/**
 * This ISR is used to handle IRQ triggers from the front panel.
 *
 * The front panel pulls the trigger pin low when a new event
 * is available. All we do here to handle the interrupt, is
 * increment a simple queue length counter. Reading the event
 * from the I2C bus will be handled in the main loop, based
 * on this counter.
 */
void ICACHE_RAM_ATTR HOT FrontPanelHAL::isr(FrontPanelHAL *store) {
    store->queue_length++;
}

} // namespace bs2
} // namespace yeelight
} // namespace esphome
