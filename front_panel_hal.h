#pragma once 

#include <array>
#include "common.h"
#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace yeelight {
namespace bs2 {

static const uint8_t MSG_LEN = 7;
using MSG = uint8_t[7];
static const MSG READY_FOR_EV = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
static const MSG TURN_ON      = { 0x02, 0x03, 0x5E, 0x00, 0x64, 0x00, 0x00 };
static const MSG TURN_OFF     = { 0x02, 0x03, 0x0C, 0x00, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_1  = { 0x02, 0x03, 0x5E, 0x00, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_2  = { 0x02, 0x03, 0x5F, 0x00, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_3  = { 0x02, 0x03, 0x5F, 0x80, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_4  = { 0x02, 0x03, 0x5F, 0xC0, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_5  = { 0x02, 0x03, 0x5F, 0xE0, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_6  = { 0x02, 0x03, 0x5F, 0xF0, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_7  = { 0x02, 0x03, 0x5F, 0xF8, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_8  = { 0x02, 0x03, 0x5F, 0xFC, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_9  = { 0x02, 0x03, 0x5F, 0xFE, 0x64, 0x00, 0x00 };
static const MSG SET_LEVEL_10 = { 0x02, 0x03, 0x5F, 0xFF, 0x64, 0x00, 0x00 };

enum FrontPanelButton {
    ButtonUnknown,
    ButtonPower,
    ButtonColor,
    ButtonSlider
};

enum FrontPanelEventType {
    TypeUnknown,
    TypeTouch,
    TypeRelease
};

class FrontPanelEvent {
public:
    bool valid;
    FrontPanelEventType type;
    FrontPanelButton button;
    uint8_t level;
    MSG message;

    void parse(uint8_t *m) {
        memcpy(message, m, MSG_LEN);
        type = TypeUnknown;
        button = ButtonUnknown;
        valid = true;
        level = 0;
        
        // All events start with 04:04:01:00.
        if (m[0] != 0x04 || m[1] != 0x04 || m[2] != 0x01 || m[3] != 0x00) {
            valid = false;
            return;
        }

        // Next byte determines the button that is touched.
        // All remaining bytes determine the event for that button.
        switch (m[4]) {
            case 0x01:
                button = ButtonPower;
                if (m[5] == 0x01 && m[6] == 0x03) {
                    type = TypeTouch;
                } else if (m[5] == 0x02 && m[6] == 0x04) {
                    type = TypeRelease;
                } else {
                    valid = false;
                }
                break;
            case 0x02:
                button = ButtonColor;
                if (m[5] == 0x01 && m[6] == 0x04) {
                    type = TypeTouch;
                } else if (m[5] == 0x02 && m[6] == 0x05) {
                    type = TypeRelease;
                } else {
                    valid = false;
                }
                break;
            case 0x03:
            case 0x04:
                button = ButtonSlider;
                type = m[4] == 0x03 ? TypeTouch : TypeRelease;
                if ((m[6] - m[5] - m[4] - 0x01) != 0) {
                    valid = false;
                } else if (m[5] > 0x16 || m[5] < 0x01) {
                    valid = false;
                } else {
                    level = 0x17 - m[5];
                }
                break;
            default:
                valid = false;
                return;
        }
    }

    void log() {
        if (button == ButtonSlider) {
            ESP_LOGI(TAG, "Event %0x:%0x:%0x:%0x:%0x:%0x:%0x => ok=%s, button=%s, type=%s, level=%d",
                message[0], message[1], message[2], message[3], message[4],
                message[5], message[6],
                (valid ? "Y" : "N"), button_str_(), type_str_(), level);
        } else {
            ESP_LOGI(TAG, "Event %0x:%0x:%0x:%0x:%0x:%0x:%0x => ok=%s, button=%s, type=%s",
                message[0], message[1], message[2], message[3], message[4],
                message[5], message[6],
                (valid ? "Y" : "N"), button_str_(), type_str_());
        }
    }
protected:
    const char *button_str_() {
        switch (button) {
            case ButtonPower: return "POWER"; break;
            case ButtonColor: return "COLOR"; break;
            case ButtonSlider: return "SLIDER"; break; 
            default: return "ERROR"; break;
        }
    }

    const char *type_str_() {
        switch (type) {
            case TypeTouch: return "TOUCH"; break;
            case TypeRelease: return "RELEASE"; break;
            default: return "ERROR";
        }
    }
};

class FrontPanelHAL : public Component, public i2c::I2CDevice {
public:
    FrontPanelEvent ev;

    void set_trigger_pin(GPIOPin *pin) { trigger_pin_ = pin; }

    void setup() {
        ESP_LOGCONFIG(TAG, "Setting up I2C trigger pin interrupt...");
        trigger_pin_->setup();
        trigger_pin_->attach_interrupt(
            FrontPanelHAL::isr, this, FALLING);
    }

    void dump_config() {
        ESP_LOGCONFIG(TAG, "I2C");
        LOG_PIN("  Interrupt pin: ", trigger_pin_);
    }

    void loop() {
        if (queue_length_ > 0) {
            queue_length_ = 0;
            read_event_();
        }
    }

protected:
    // The GPIO pin that is used by the front panel to notify the ESP that
    // a touch/release event can be read using I2C.
    GPIOPin *trigger_pin_;

    // The ISR that is used for handling event interrupts.
    static void isr(FrontPanelHAL *store);

    // The number of unhandled event interrupts.
    volatile int queue_length_ = 0;

    uint8_t message[MSG_LEN];

    void read_event_() {
        if (!write_bytes_raw(READY_FOR_EV, MSG_LEN)) {
            return;
        }
        if (!read_bytes_raw(message, MSG_LEN)) {
            return;
        }

        ev.parse(message);
        ev.log();
    }
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
    store->queue_length_++;
}

} // namespace bs2
} // namespace yeelight
} // namespace esphome
