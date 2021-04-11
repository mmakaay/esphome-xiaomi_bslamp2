#pragma once 

#include "common.h"
#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/ledc/ledc_output.h"
#include "esphome/components/gpio/output/gpio_binary_output.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

struct TriggerPinStore {
    volatile int queue_length = 0;
    static void isr(TriggerPinStore *store);
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
void ICACHE_RAM_ATTR HOT TriggerPinStore::isr(TriggerPinStore *store) {
    store->queue_length++;
}

class YeelightBS2Hub : public Component {
public:
    // Pins that are used for the RGBWW light.
    ledc::LEDCOutput *red;
    ledc::LEDCOutput *green;
    ledc::LEDCOutput *blue;
    ledc::LEDCOutput *white;
    gpio::GPIOBinaryOutput *master1;
    gpio::GPIOBinaryOutput *master2;

    void set_trigger_pin(GPIOPin *pin) { i2c_trigger_pin_ = pin; }
    void set_red_pin(ledc::LEDCOutput *pin) { red = pin; }
    void set_green_pin(ledc::LEDCOutput *pin) { green = pin; }
    void set_blue_pin(ledc::LEDCOutput *pin) { blue = pin; }
    void set_white_pin(ledc::LEDCOutput *pin) { white = pin; }
    void set_master1_pin(gpio::GPIOBinaryOutput *pin) { master1 = pin; }
    void set_master2_pin(gpio::GPIOBinaryOutput *pin) { master2 = pin; }
    void set_front_panel_i2c(i2c::I2CComponent *fp_i2c) { fp_i2c_ = fp_i2c; }

    void setup() {
        ESP_LOGCONFIG(TAG, "Setting up I2C trigger pin interrupt...");
        this->i2c_trigger_pin_->setup();
        this->i2c_trigger_pin_->attach_interrupt(
            TriggerPinStore::isr, &this->trigger_pin_store_, FALLING);
    }

    void dump_config() {
        ESP_LOGCONFIG(TAG, "I2C");
        LOG_PIN("  Interrupt pin: ", this->i2c_trigger_pin_);
    }

    void loop() {
        if (this->trigger_pin_store_.queue_length > 0) {
            this->counter_++;
            ESP_LOGD(TAG, "Front Panel interrupt queue=%d, counter=%d",
                this->trigger_pin_store_.queue_length, this->counter_);
            this->trigger_pin_store_.queue_length--;
        }
    }

protected:
    // Pin that is used by the front panel to notify the ESP that
    // a touch/release event can be read using I2C.
    GPIOPin *i2c_trigger_pin_;

    // The I2C bus that is connected to the front panel.
    i2c::I2CComponent *fp_i2c_;

    // Fields that are used for trigger pin interrupt handling.
    int counter_ = 0;
    TriggerPinStore trigger_pin_store_{};
};
    
} // namespace bs2
} // namespace yeelight
} // namespace esphome
