import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components.ledc.output import LEDCOutput
from esphome.components.gpio.output import GPIOBinaryOutput
from esphome.components.i2c import I2CComponent, I2CDevice
from esphome.core import coroutine
from esphome.core import CORE
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_TRIGGER_PIN,
    CONF_SDA, CONF_SCL, CONF_ADDRESS, CONF_PLATFORM
)

CODEOWNERS = ["@mmakaay"]

CONF_HUB_ID = "xiaomi_bslamp2_hub_id"
CONF_RED_ID = "red_id"
CONF_GREEN_ID = "green_id"
CONF_BLUE_ID = "blue_id"
CONF_WHITE_ID = "white_id"
CONF_MASTER1 = "master1"
CONF_MASTER1_ID = "master1_id"
CONF_MASTER2 = "master2"
CONF_MASTER2_ID = "master2_id"
CONF_FP_I2C_ID = "front_panel_i2c_id"
CONF_LIGHT_HAL_ID = "light_hal"
CONF_FRONT_PANEL_HAL_ID = "front_panel_hal_id"

CONF_ON_BRIGHTNESS = "on_brightness"

AUTO_LOAD = ["ledc", "output", "i2c"]

xiaomi_ns = cg.esphome_ns.namespace("xiaomi")
bslamp2_ns = xiaomi_ns.namespace("bslamp2")
LightHAL = bslamp2_ns.class_("LightHAL", cg.Component)
FrontPanelHAL = bslamp2_ns.class_("FrontPanelHAL", cg.Component, I2CDevice)

def make_config_schema():
    schema = cv.COMPONENT_SCHEMA.extend({
        # RGBWW Light
        cv.GenerateID(CONF_LIGHT_HAL_ID): cv.declare_id(LightHAL),
        cv.GenerateID(CONF_RED_ID): cv.declare_id(LEDCOutput),
        cv.Optional(CONF_RED, default="GPIO13"): pins.validate_gpio_pin,
        cv.GenerateID(CONF_GREEN_ID): cv.declare_id(LEDCOutput),
        cv.Optional(CONF_GREEN, default="GPIO14"): pins.validate_gpio_pin,
        cv.GenerateID(CONF_BLUE_ID): cv.declare_id(LEDCOutput),
        cv.Optional(CONF_BLUE, default="GPIO5"): pins.validate_gpio_pin,
        cv.GenerateID(CONF_WHITE_ID): cv.declare_id(LEDCOutput),
        cv.Optional(CONF_WHITE, default="GPIO12"): pins.validate_gpio_pin,
        cv.GenerateID(CONF_MASTER1_ID): cv.declare_id(GPIOBinaryOutput),
        cv.Optional(CONF_MASTER1, default="GPIO33"): pins.validate_gpio_pin,
        cv.GenerateID(CONF_MASTER2_ID): cv.declare_id(GPIOBinaryOutput),
        cv.Optional(CONF_MASTER2, default="GPIO4"): pins.validate_gpio_pin,

        # Front panel I2C
        cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.declare_id(FrontPanelHAL),
        cv.GenerateID(CONF_FP_I2C_ID): cv.use_id(I2CComponent),
        cv.Optional(CONF_SDA, default="GPIO21"): pins.validate_gpio_pin,
        cv.Optional(CONF_SCL, default="GPIO19"): pins.validate_gpio_pin,
        cv.Optional(CONF_ADDRESS, default="0x2C"): cv.i2c_address,
        cv.Optional(CONF_TRIGGER_PIN, default="GPIO16"): cv.All(
            pins.validate_gpio_pin,
            pins.validate_has_interrupt
        ),
    })

    return schema;

CONFIG_SCHEMA = make_config_schema()

@coroutine
def make_gpio(number, mode="OUTPUT"):
    yield from cg.gpio_pin_expression({ "number": number, "mode": mode });

@coroutine
def make_gpio_binary_output(id_, number):
    gpio_var = yield make_gpio(number)
    output_var = cg.new_Pvariable(id_)
    cg.add(output_var.set_pin(gpio_var))   
    yield from cg.register_component(output_var, {})

@coroutine
def make_ledc_output(id_, number, frequency, channel):
    gpio_var = yield make_gpio(number)
    ledc_var = cg.new_Pvariable(id_, gpio_var)
    cg.add(ledc_var.set_frequency(frequency));
    cg.add(ledc_var.set_channel(channel));
    yield from cg.register_component(ledc_var, {})

@coroutine
def make_light_hal(config):
    r_var = yield make_ledc_output(config[CONF_RED_ID], config[CONF_RED], 3000, 0)
    g_var = yield make_ledc_output(config[CONF_GREEN_ID], config[CONF_GREEN], 3000, 1)
    b_var = yield make_ledc_output(config[CONF_BLUE_ID], config[CONF_BLUE], 3000, 2)
    w_var = yield make_ledc_output(config[CONF_WHITE_ID], config[CONF_WHITE], 10000, 4)
    m1_var = yield make_gpio_binary_output(config[CONF_MASTER1_ID], config[CONF_MASTER1])
    m2_var = yield make_gpio_binary_output(config[CONF_MASTER2_ID], config[CONF_MASTER2])
    light_hal = cg.new_Pvariable(config[CONF_LIGHT_HAL_ID])
    yield cg.register_component(light_hal, config)
    cg.add(light_hal.set_red_pin(r_var))
    cg.add(light_hal.set_green_pin(g_var))
    cg.add(light_hal.set_blue_pin(b_var))
    cg.add(light_hal.set_white_pin(w_var))
    cg.add(light_hal.set_master1_pin(m1_var))
    cg.add(light_hal.set_master2_pin(m2_var))

@coroutine
def make_front_panel_hal(config):
    trigger_pin = yield make_gpio(config[CONF_TRIGGER_PIN], "INPUT")
    fp_hal = cg.new_Pvariable(config[CONF_FRONT_PANEL_HAL_ID])
    yield cg.register_component(fp_hal, config)
    cg.add(fp_hal.set_trigger_pin(trigger_pin))   

    # The i2c component automatically sets up one I2C bus.
    # Take that bus and update is to make it work for the
    # front panel I2C communication.
    fp_i2c_var = yield cg.get_variable(config[CONF_FP_I2C_ID])
    cg.add(fp_i2c_var.set_sda_pin(config[CONF_SDA]))
    cg.add(fp_i2c_var.set_scl_pin(config[CONF_SCL]))
    cg.add(fp_i2c_var.set_scan(True))
    cg.add(fp_hal.set_i2c_parent(fp_i2c_var))
    cg.add(fp_hal.set_i2c_address(config[CONF_ADDRESS]))

def to_code(config):
    # Dirty little hack to make the ESPHome component loader include
    # the code for the "gpio" platform for the "output" domain.
    # Loading specific platform components is not possible using
    # the AUTO_LOAD feature unfortunately.
    CORE.config["output"].append({ CONF_PLATFORM: "gpio" })

    yield make_light_hal(config)
    yield make_front_panel_hal(config)
