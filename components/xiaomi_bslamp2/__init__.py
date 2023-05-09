import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components.ledc.output import LEDCOutput
from esphome.components.gpio.output import GPIOBinaryOutput
from esphome.components.i2c import I2CBus, I2CDevice
from esphome.components.esp32 import gpio_esp32
from esphome.const import (
    CONF_LIGHT, CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE,
    CONF_I2C, CONF_ADDRESS, CONF_TRIGGER_PIN, CONF_ID
)
from esphome.util import parse_esphome_version
from voluptuous import Invalid

CODEOWNERS = ["@mmakaay"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_FRONT_PANEL = "front_panel"
CONF_LIGHT_HAL_ID = "light_hal_id"
CONF_FRONT_PANEL_HAL_ID = "front_panel_hal_id"

xiaomi_ns = cg.esphome_ns.namespace("xiaomi")
bslamp2_ns = xiaomi_ns.namespace("bslamp2")
LightHAL = bslamp2_ns.class_("LightHAL", cg.Component)
FrontPanelHAL = bslamp2_ns.class_("FrontPanelHAL", cg.Component, I2CDevice)

FrontPanelLEDs = bslamp2_ns.enum("FrontPanelLEDs")
FRONT_PANEL_LED_OPTIONS = {
    "NONE": FrontPanelLEDs.LED_NONE,
    "ALL": FrontPanelLEDs.LED_ALL,
    "POWER": FrontPanelLEDs.LED_POWER,
    "COLOR": FrontPanelLEDs.LED_COLOR,
    "1": FrontPanelLEDs.LED_1,
    "2": FrontPanelLEDs.LED_2,
    "3": FrontPanelLEDs.LED_3,
    "4": FrontPanelLEDs.LED_4,
    "5": FrontPanelLEDs.LED_5,
    "6": FrontPanelLEDs.LED_6,
    "7": FrontPanelLEDs.LED_7,
    "8": FrontPanelLEDs.LED_8,
    "9": FrontPanelLEDs.LED_9,
    "10": FrontPanelLEDs.LED_10,
}


def check_version_compatibility(config):
    esphome_version = parse_esphome_version()
    if esphome_version < (2023, 4, 0):
        raise Invalid(
            "This xiaomi_bslamp2 component requires at least ESPHome " +
            "version 2023.4.0; Please upgrade ESPHome and try again."
        )
    return config


CONFIG_SCHEMA = cv.All(
  check_version_compatibility,
  cv.COMPONENT_SCHEMA.extend({
      # RGBWW Light
      cv.Required(CONF_LIGHT): cv.Schema(
          {
              cv.GenerateID(CONF_LIGHT_HAL_ID): cv.declare_id(LightHAL),
              cv.Required(CONF_RED): cv.use_id(LEDCOutput),
              cv.Required(CONF_GREEN): cv.use_id(LEDCOutput),
              cv.Required(CONF_BLUE): cv.use_id(LEDCOutput),
              cv.Required(CONF_WHITE): cv.use_id(LEDCOutput),
              cv.Required(CONF_MASTER1): cv.use_id(GPIOBinaryOutput),
              cv.Required(CONF_MASTER2): cv.use_id(GPIOBinaryOutput),
          }
      ),
      # Front panel I2C
      cv.Required(CONF_FRONT_PANEL): cv.Schema(
          {
              cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.declare_id(FrontPanelHAL),
              cv.Required(CONF_I2C): cv.use_id(I2CBus),
              cv.Required(CONF_ADDRESS): cv.i2c_address,
              cv.Required(CONF_TRIGGER_PIN): cv.All(pins.internal_gpio_input_pin_schema)
          }
      ),
  })
)

async def make_light_hal(config):
    light_hal = cg.new_Pvariable(config[CONF_LIGHT][CONF_LIGHT_HAL_ID])
    await cg.register_component(light_hal, config)
    cg.add(light_hal.set_red_pin(await cg.get_variable(config[CONF_LIGHT][CONF_RED])))
    cg.add(light_hal.set_green_pin(await cg.get_variable(config[CONF_LIGHT][CONF_GREEN])))
    cg.add(light_hal.set_blue_pin(await cg.get_variable(config[CONF_LIGHT][CONF_BLUE])))
    cg.add(light_hal.set_white_pin(await cg.get_variable(config[CONF_LIGHT][CONF_WHITE])))
    cg.add(light_hal.set_master1_pin(await cg.get_variable(config[CONF_LIGHT][CONF_MASTER1])))
    cg.add(light_hal.set_master2_pin(await cg.get_variable(config[CONF_LIGHT][CONF_MASTER2])))

async def make_front_panel_hal(config):
    fp_hal = cg.new_Pvariable(config[CONF_FRONT_PANEL][CONF_FRONT_PANEL_HAL_ID])
    await cg.register_component(fp_hal, config)
    trigger_pin = await cg.gpio_pin_expression(config[CONF_FRONT_PANEL][CONF_TRIGGER_PIN])
    cg.add(fp_hal.set_trigger_pin(trigger_pin))   

    fp_i2c_var = await cg.get_variable(config[CONF_FRONT_PANEL][CONF_I2C])
    cg.add(fp_hal.set_i2c_bus(fp_i2c_var))
    cg.add(fp_hal.set_i2c_address(config[CONF_FRONT_PANEL][CONF_ADDRESS]))


# Clear the configuration of strapping pins in ESPHome, to suppress pin
# usage warnings that often confuse users of this firmware (when there
# are problems, these often pop up as "is this the issue?").
# The hardware on the lamp is as-is, and warnings about pins that might
# better not be used are futile.
gpio_esp32._ESP32_STRAPPING_PINS = {}


async def to_code(config):
    await make_light_hal(config)
    await make_front_panel_hal(config)
