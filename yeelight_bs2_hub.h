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

class LightHAL {
public:
    virtual void light_turn_on() = 0;
    virtual void light_turn_off() = 0;
    virtual void light_set_rgbw(float r, float g, float b, float w) = 0;
};

class YeelightBS2Hub : public Component, public LightHAL {
public:
    void set_trigger_pin(GPIOPin *pin) { i2c_trigger_pin_ = pin; }
    void set_red_pin(ledc::LEDCOutput *pin) { red_ = pin; }
    void set_green_pin(ledc::LEDCOutput *pin) { green_ = pin; }
    void set_blue_pin(ledc::LEDCOutput *pin) { blue_ = pin; }
    void set_white_pin(ledc::LEDCOutput *pin) { white_ = pin; }
    void set_master1_pin(gpio::GPIOBinaryOutput *pin) { master1_ = pin; }
    void set_master2_pin(gpio::GPIOBinaryOutput *pin) { master2_ = pin; }
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

    void light_turn_on() {
        master1_->turn_on();
        master2_->turn_on();
    }

    void light_turn_off() {
        master1_->turn_off();
        master2_->turn_off();
    }

    void light_set_rgbw(float r, float g, float b, float w) {
        red_->set_level(r);
        green_->set_level(g);
        blue_->set_level(b);
        white_->set_level(w);
    }

protected:
    // Pins that are used for the RGBWW LEDs.
    ledc::LEDCOutput *red_;
    ledc::LEDCOutput *green_;
    ledc::LEDCOutput *blue_;
    ledc::LEDCOutput *white_;
    gpio::GPIOBinaryOutput *master1_;
    gpio::GPIOBinaryOutput *master2_;

    // Pin that is used by the front panel to notify the ESP that
    // a touch/release event can be read using I2C.
    GPIOPin *i2c_trigger_pin_;

    // The I2C bus that is connected to the front panel.
    i2c::I2CComponent *fp_i2c_;

    // Fields that are used for trigger pin interrupt handling.
    int counter_ = 0;
    TriggerPinStore trigger_pin_store_{};

    friend class LightHAL;
};

} // namespace bs2
} // namespace yeelight
} // namespace esphome
