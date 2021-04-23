#pragma once

#include "common.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include <array>

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

static const uint8_t MSG_LEN = 7;
using MSG = uint8_t[7];

// clang-format off

// The commands that are supported by the front panel component.
static const MSG READY_FOR_EV = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const MSG TURN_ON      = {0x02, 0x03, 0x5E, 0x00, 0x64, 0x00, 0x00};
static const MSG TURN_OFF     = {0x02, 0x03, 0x0C, 0x00, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_1  = {0x02, 0x03, 0x5E, 0x00, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_2  = {0x02, 0x03, 0x5F, 0x00, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_3  = {0x02, 0x03, 0x5F, 0x80, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_4  = {0x02, 0x03, 0x5F, 0xC0, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_5  = {0x02, 0x03, 0x5F, 0xE0, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_6  = {0x02, 0x03, 0x5F, 0xF0, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_7  = {0x02, 0x03, 0x5F, 0xF8, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_8  = {0x02, 0x03, 0x5F, 0xFC, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_9  = {0x02, 0x03, 0x5F, 0xFE, 0x64, 0x00, 0x00};
static const MSG SET_LEVEL_10 = {0x02, 0x03, 0x5F, 0xFF, 0x64, 0x00, 0x00};

using EVENT = uint16_t;

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
      return error_(ev, m, "prefix is not 04:04:01:00");
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
          return error_(ev, m, "invalid event type for button");
        break;
      case 0x03:  // slider touch
      case 0x04:  // slider release
        ev |= FLAG_PART_SLIDER;
        ev |= (m[4] == 0x03 ? FLAG_TYPE_TOUCH : FLAG_TYPE_RELEASE);
        if ((m[6] - m[5] - m[4] - 0x01) != 0)
          return error_(ev, m, "invalid slider level crc");
        else if (m[5] > 0x16 || m[5] < 0x01)
          return error_(ev, m, "out of bounds slider value");
        else {
          auto level = 0x17 - m[5];
          ev |= (level << FLAG_LEVEL_SHIFT);
        }
        break;
      default:
        return error_(ev, m, "invalid part id");
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
    ESP_LOGE(TAG, "  Parsed part: %s", format_part(ev));
    ESP_LOGE(TAG, "  Parsed event type: %s", format_event_type(ev));
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_SLIDER)) {
      auto level = (ev & FLAG_LEVEL_MASK) >> FLAG_LEVEL_SHIFT;
      if (level > 0) {
        ESP_LOGE(TAG, "  Parsed slider level: %d", level);
      }
    }

    return ev;
  }

  const char *format_part(EVENT ev) {
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_POWER))
      return "power button";
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_COLOR))
      return "color button";
    if (has_(ev, FLAG_PART_MASK, FLAG_PART_SLIDER))
      return "slider";
    return "n/a";
  }

  const char *format_event_type(EVENT ev) {
    if (has_(ev, FLAG_TYPE_MASK, FLAG_TYPE_TOUCH))
      return "touch";
    if (has_(ev, FLAG_TYPE_MASK, FLAG_TYPE_RELEASE))
      return "release";
    return "n/a";
  }
};

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
  void set_trigger_pin(GPIOPin *pin) { trigger_pin_ = pin; }

  void add_on_event_callback(std::function<void(EVENT)> &&callback) { event_callback_.add(std::move(callback)); }

  void setup() {
    ESP_LOGCONFIG(TAG, "Setting up I2C trigger pin interrupt...");
    trigger_pin_->setup();
    trigger_pin_->attach_interrupt(FrontPanelHAL::isr, this, FALLING);
  }

  void dump_config() {
    ESP_LOGCONFIG(TAG, "FrontPanelHAL:");
    LOG_PIN("  I2C interrupt pin: ", trigger_pin_);
  }

  void loop() {
    // Read and publish front panel events.
    auto current_event_id = event_id_;
    if (current_event_id != last_event_id_) {
      last_event_id_ = current_event_id;
      MSG message;
      if (write_bytes_raw(READY_FOR_EV, MSG_LEN) && read_bytes_raw(message, MSG_LEN)) {
        auto ev = event.parse(message);
        if (ev & FLAG_OK) {
          event_callback_.call(ev);
        }
      }
    }
  }

  /**
   * Sets the front panel illumination to the provided level (0.0 - 1.0).
   *
   * Level 0.0 means: turn off the front panel illumination.
   * The other levels are translated to one of the available levels,
   * represented by the level indicator (i.e. the illumination of the
   * slider bar.)
   */
  void set_light_level(float level) {
    if (level == 0.0f)
      write_bytes_raw(TURN_OFF, MSG_LEN);
    else if (level < 0.15)
      write_bytes_raw(SET_LEVEL_1, MSG_LEN);
    else if (level < 0.25)
      write_bytes_raw(SET_LEVEL_2, MSG_LEN);
    else if (level < 0.35)
      write_bytes_raw(SET_LEVEL_3, MSG_LEN);
    else if (level < 0.45)
      write_bytes_raw(SET_LEVEL_4, MSG_LEN);
    else if (level < 0.55)
      write_bytes_raw(SET_LEVEL_5, MSG_LEN);
    else if (level < 0.65)
      write_bytes_raw(SET_LEVEL_6, MSG_LEN);
    else if (level < 0.75)
      write_bytes_raw(SET_LEVEL_7, MSG_LEN);
    else if (level < 0.85)
      write_bytes_raw(SET_LEVEL_8, MSG_LEN);
    else if (level < 0.95)
      write_bytes_raw(SET_LEVEL_9, MSG_LEN);
    else
      write_bytes_raw(SET_LEVEL_10, MSG_LEN);
  }

 protected:
  GPIOPin *trigger_pin_;
  static void isr(FrontPanelHAL *store);
  volatile int event_id_ = 0;
  int last_event_id_ = 0;
  CallbackManager<void(EVENT)> event_callback_{};
};

/**
 * This ISR is used to handle IRQ triggers from the front panel.
 *
 * The front panel pulls the trigger pin low for a short period of time
 * when a new event is available. All we do here to handle the interrupt,
 * is increment a simple event id counter. The main loop of the component
 * will take care of actually reading and processing the event.
 */
void ICACHE_RAM_ATTR HOT FrontPanelHAL::isr(FrontPanelHAL *store) { store->event_id_++; }

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
