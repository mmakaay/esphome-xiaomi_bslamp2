#pragma once

#include "common.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <array>
#include <cmath>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

static const uint8_t MSG_LEN = 7;
using MSG = uint8_t[MSG_LEN];
using LED = uint16_t;
using EVENT = uint16_t;

// clang-format off

// Bit flags that are used for indicating the LEDs in the front panel. 
// LED_1 is the slider LED closest to the power button.
// LED_10 is the one closest to the color button.
enum FrontPanelLEDs {
  LED_ALL        = 16384 + 4096 + 1023,
  LED_ALL_SLIDER = 512 + 256 + 128 + 64 + 32 + 16 + 8 + 4 + 2 + 1,
  LED_POWER      = 16384,
  LED_COLOR      = 4096,
  LED_1          = 512,
  LED_2          = 256,
  LED_3          = 128,
  LED_4          = 64,
  LED_5          = 32,
  LED_6          = 16,
  LED_7          = 8,
  LED_8          = 4,
  LED_9          = 2,
  LED_10         = 1,
  LED_NONE       = 0,
};

// This I2C command is used during front panel event handling.
static const MSG READY_FOR_EV = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

// Bit flags that are used for specifying an event.
// Events are registered using the following bit pattern
// (bit 1 being the least significant bit):
//
// BITS  INDICATE  PATTERN  RESULT
// 1     status    0        parsing event failed
//                 1        parsing event successful
// 2-4   part      000      part unknown
//                 001      power button
//                 010      color button
//                 100      slider
// 5-6   type      00       type unknown
//                 01       touch
//                 10       release
// 7-11  slider    00000    level known (or part is not "slider")
//       level     00001    level 1
//                  ...     up to
//                 10101    level 21
//
static const EVENT FLAG_INIT          = 0b00000000000;

static const EVENT FLAG_ERR           = 0b00000000000;
static const EVENT FLAG_OK            = 0b00000000001;

static const EVENT FLAG_PART_SHIFT    = 1;
static const EVENT FLAG_PART_MASK     = 0b00000001110;
static const EVENT FLAG_PART_UNKNOWN  = 0b00000000000;
static const EVENT FLAG_PART_POWER    = 0b00000000010;
static const EVENT FLAG_PART_COLOR    = 0b00000000100;
static const EVENT FLAG_PART_SLIDER   = 0b00000001000;

static const EVENT FLAG_TYPE_SHIFT    = 4;
static const EVENT FLAG_TYPE_MASK     = 0b00000110000;
static const EVENT FLAG_TYPE_UNKNOWN  = 0b00000000000;
static const EVENT FLAG_TYPE_TOUCH    = 0b00000010000;
static const EVENT FLAG_TYPE_RELEASE  = 0b00000100000;

static const EVENT FLAG_LEVEL_SHIFT   = 6;
static const EVENT FLAG_LEVEL_MASK    = 0b11111000000;
static const EVENT FLAG_LEVEL_UNKNOWN = 0b00000000000;

// clang-format on

/**
 * This class implements a parser that translates event byte codes from the
 * Xiaomi Mijia Bedside Lamp 2 into usable events.
 */
class FrontPanelEventParser {
 public:
  /**
   * Parse the provided event byte code (7 bytes long).
   * Returns a unique integer event code that describes the parsed event.
   */
  EVENT parse(uint8_t *m) {
    EVENT ev = FLAG_INIT;

    // All events use the prefix [04:04:01:00].
    if (m[0] != 0x04 || m[1] != 0x04 || m[2] != 0x01 || m[3] != 0x00) {
      return this->error_(ev, m, "prefix is not 04:04:01:00");
    }

    // The next byte determines the part that is touched.
    // All remaining bytes specify the event for that part.
    switch (m[4]) {
      case 0x01:  // power button
      case 0x02:  // color button
        ev |= (m[4] == 0x01 ? FLAG_PART_POWER : FLAG_PART_COLOR);
        if (m[5] == 0x01 && m[6] == (0x02 + m[4]))
          ev |= FLAG_TYPE_TOUCH;
        else if (m[5] == 0x02 && m[6] == (0x03 + m[4]))
          ev |= FLAG_TYPE_RELEASE;
        else
          return this->error_(ev, m, "invalid event type for button");
        break;
      case 0x03:  // slider touch
      case 0x04:  // slider release
        ev |= FLAG_PART_SLIDER;
        ev |= (m[4] == 0x03 ? FLAG_TYPE_TOUCH : FLAG_TYPE_RELEASE);
        if ((m[6] - m[5] - m[4] - 0x01) != 0)
          return this->error_(ev, m, "invalid slider level crc");
        else if (m[5] > 0x16 || m[5] < 0x01)
          return this->error_(ev, m, "out of bounds slider value");
        else {
          auto level = 0x17 - m[5];
          ev |= (level << FLAG_LEVEL_SHIFT);
        }
        break;
      default:
        return this->error_(ev, m, "invalid part id");
        return ev;
    }

    // All parsing rules passed. This event is valid.
    ESP_LOGD(TAG, "Front panel I2C event parsed: code=%d", ev);
    ev |= FLAG_OK;

    return ev;
  }

 protected:
  bool has_(EVENT ev, EVENT mask, EVENT flag) { return (ev & mask) == flag; }

  EVENT error_(EVENT ev, uint8_t *m, const char *msg) {
    ESP_LOGE(TAG, "Front panel I2C event error:");
    ESP_LOGE(TAG, "  Error: %s", msg);
    ESP_LOGE(TAG, "  Event: [%02x:%02x:%02x:%02x:%02x:%02x:%02x]", m[0], m[1], m[2], m[3], m[4], m[5], m[6]);
    ESP_LOGE(TAG, "  Parsed part: %s", this->format_part_(ev));
    ESP_LOGE(TAG, "  Parsed event type: %s", this->format_event_type_(ev));
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_SLIDER)) {
      auto level = (ev & FLAG_LEVEL_MASK) >> FLAG_LEVEL_SHIFT;
      if (level > 0) {
        ESP_LOGE(TAG, "  Parsed slider level: %d", level);
      }
    }

    return ev;
  }

  const char *format_part_(EVENT ev) {
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_POWER))
      return "power button";
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_COLOR))
      return "color button";
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_SLIDER))
      return "slider";
    return "n/a";
  }

  const char *format_event_type_(EVENT ev) {
    if (has_(ev, FLAG_TYPE_MASK, FLAG_TYPE_TOUCH))
      return "touch";
    if (has_(ev, FLAG_TYPE_MASK, FLAG_TYPE_RELEASE))
      return "release";
    return "n/a";
  }
};

struct FrontPanelTriggerStore {
  volatile int event_id{0};
  static void gpio_intr(FrontPanelTriggerStore *store);
};

/**
 * This ISR is used to handle IRQ triggers from the front panel.
 *
 * The front panel pulls the trigger pin low for a short period of time
 * when a new event is available. All we do here to handle the interrupt,
 * is increment a simple event id counter. The main loop of the component
 * will take care of actually reading and processing the event.
 */
void IRAM_ATTR HOT FrontPanelTriggerStore::gpio_intr(FrontPanelTriggerStore *store) {
  store->event_id++;
}

/**
 * This is a hardware abstraction layer that communicates with with front
 * panel of the Xiaomi Mijia Bedside Lamp 2.
 *
 * It serves as a hub component for other components that implement
 * the actual buttons and slider components.
 */
class FrontPanelHAL : public Component, public i2c::I2CDevice {
 public:
  FrontPanelEventParser event;

  /**
   * Set the GPIO pin that is used by the front panel to notify the ESP
   * that a touch/release event can be read using I2C.
   */
  void set_trigger_pin(InternalGPIOPin *pin) {
    trigger_pin_ = pin;
  }

  void add_on_event_callback(std::function<void(EVENT)> &&callback) {
    event_callback_.add(std::move(callback));
  }

  void setup() {
    ESP_LOGCONFIG(TAG, "Setting up I2C trigger pin interrupt...");
    this->trigger_pin_->setup();
    this->trigger_pin_->attach_interrupt(
      FrontPanelTriggerStore::gpio_intr,
      &this->store_,
      gpio::INTERRUPT_FALLING_EDGE);
  }

  void dump_config() {
    ESP_LOGCONFIG(TAG, "FrontPanelHAL:");
    LOG_I2C_DEVICE(this);
    LOG_PIN("  I2C interrupt pin: ", trigger_pin_);
  }

  void loop() {
    // Read and publish front panel events.
    auto current_event_id = this->store_.event_id;
    if (current_event_id != this->last_event_id_) {
      this->last_event_id_ = current_event_id;
      if (this->write(READY_FOR_EV, MSG_LEN) != i2c::ERROR_OK) {
        ESP_LOGW(TAG, "Writing READY_FOR_EV to front panel failed");
      }
      MSG message;
      if (this->read(message, MSG_LEN) != i2c::ERROR_OK) {
        ESP_LOGW(TAG, "Reading message from front panel failed");
        return;
      }
      auto ev = event.parse(message);
      if (ev & FLAG_OK) {
        this->event_callback_.call(ev);
      } else {
        ESP_LOGW(TAG, "Skipping unsupported message from front panel");
      }
    }

    if (led_state_ != last_led_state_) {
      update_leds();
    }
  }

  /**
   * Turn on one or more LEDs (leaving the state of the other LEDs intact).
   * The input value is a bitwise OR-ed set of LED constants.
   * Only after a call to update_leds() (handled by default from the main loop),
   * the new state will be activated.
   */
  void turn_on_leds(uint16_t leds) {
    led_state_ = led_state_ | 0b0000110000000000 | leds;
  }

  /**
   * Turn off one or more LEDs (leaving the state of the other LEDs intact).
   * The input value is a bitwise OR-ed set of LED constants.
   * Only after a call to update_leds() (handled by default from the main loop),
   * the new state will be activated.
   */
  void turn_off_leds(uint16_t leds) {
    led_state_ = (led_state_ | 0b0000110000000000) & ~leds;
  }

  /**
   * Updates the state of the LEDs according to the provided input.
   * The input value is a bitwise OR-ed set of LED constants, representing the
   * LEDs that must be turned on. All other LEDs are turned off.
   * Only after a call to update_leds() (handled by default from the main loop),
   * the new state will be activated.
   */
  void set_leds(uint16_t leds) {
    turn_off_leds(LED_ALL);
    turn_on_leds(leds);
  }

  /**
   * Activate the LEDs according to the currently stored LED state. This method
   * will be called automatically by the main loop. You can call this method,
   * in case you need to update the LED state right away.
   */
  void update_leds() {
    led_msg_[2] = led_state_ >> 8;
    led_msg_[3] = led_state_ & 0xff;
    write(led_msg_, MSG_LEN);
    last_led_state_ = led_state_;
  }

  /**
   * Sets the front panel slider illumination to the provided level,
   * based on a float input (0.0 - 1.0).
   *
   * This implements the behavior of the original firmware for representing
   * the lamp's brightness.
   *
   * Level 0.0 means: turn off the slider illumination.
   * The other levels are translated to one of the available levels.
   */
  void set_slider_level(float level) {
    turn_off_leds(LED_ALL_SLIDER);
    if (level == 0.00f) return;
    if (level > 0.00f) turn_on_leds(LED_1);
    if (level > 0.15f) turn_on_leds(LED_2);
    if (level > 0.25f) turn_on_leds(LED_3);
    if (level > 0.35f) turn_on_leds(LED_4);
    if (level > 0.45f) turn_on_leds(LED_5);
    if (level > 0.55f) turn_on_leds(LED_6);
    if (level > 0.65f) turn_on_leds(LED_7);
    if (level > 0.75f) turn_on_leds(LED_8);
    if (level > 0.85f) turn_on_leds(LED_9);
    if (level > 0.95f) turn_on_leds(LED_10);
  }

 protected:
  InternalGPIOPin *trigger_pin_;
  FrontPanelTriggerStore store_{};
  int last_event_id_ = 0;
  CallbackManager<void(EVENT)> event_callback_{};

  uint16_t led_state_ = 0;
  uint16_t last_led_state_ = 0;
  MSG led_msg_ = {0x02, 0x03, 0x00, 0x00, 0x64, 0x00, 0x00};
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
