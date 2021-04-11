import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components.ledc.output import LEDCOutput, validate_frequency
from esphome.components.gpio.output import GPIOBinaryOutput
from esphome.core import coroutine
from esphome.core import CORE
from esphome.const import (
    CONF_ID,
    CONF_RED,
    CONF_GREEN,
    CONF_BLUE,
    CONF_WHITE,
    CONF_TRIGGER_PIN,
    CONF_OUTPUT_ID,
    CONF_TRIGGER_ID,
    CONF_PIN,
    CONF_FREQUENCY,
    CONF_CHANNEL,
    CONF_PLATFORM,
)

CONF_HUB_ID = "yeelight_bs2_hub_id"
CONF_RED_ID = "red_id"
CONF_GREEN_ID = "green_id"
CONF_BLUE_ID = "blue_id"
CONF_WHITE_ID = "white_id"
CONF_MASTER1 = "master1"
CONF_MASTER1_ID = "master1_id"
CONF_MASTER2 = "master2"
CONF_MASTER2_ID = "master2_id"
CONF_ON_BRIGHTNESS = "on_brightness"

CODEOWNERS = ["@mmakaay"]

AUTO_LOAD = ["ledc", "output"]

yeelight_ns = cg.esphome_ns.namespace("yeelight")
bs2_ns = yeelight_ns.namespace("bs2")
YeelightBS2Hub = bs2_ns.class_("YeelightBS2Hub", cg.Component)

PINS = {
    # Config key     TYPE,             ID               GPIO,     PARAMS
    CONF_RED     : ( LEDCOutput,       CONF_RED_ID,     "GPIO13", 3000,  0 ),
    CONF_GREEN   : ( LEDCOutput,       CONF_GREEN_ID,   "GPIO14", 3000,  1 ),
    CONF_BLUE    : ( LEDCOutput,       CONF_BLUE_ID,    "GPIO5",  3000,  2 ),
    CONF_WHITE   : ( LEDCOutput,       CONF_WHITE_ID,   "GPIO12", 10000, 4 ),
    CONF_MASTER1 : ( GPIOBinaryOutput, CONF_MASTER1_ID, "GPIO33" ),
    CONF_MASTER2 : ( GPIOBinaryOutput, CONF_MASTER2_ID, "GPIO4" ),
}

def make_config_schema():
    schema = cv.COMPONENT_SCHEMA.extend({
        cv.GenerateID(): cv.declare_id(YeelightBS2Hub),
        cv.Optional(CONF_TRIGGER_PIN, default="GPIO16"): cv.All(
            pins.validate_gpio_pin,
            pins.validate_has_interrupt
        ),
    })

    for key, pin_config in PINS.items():
        type_, id_, pin, *_ = pin_config
        schema = schema.extend({
            cv.GenerateID(id_): cv.declare_id(type_),
            cv.Optional(key, default=pin): pins.validate_gpio_pin
        })

    return schema;


CONFIG_SCHEMA = make_config_schema()

@coroutine
def make_gpio_pin(key, config):
    type_, id_, *_ = PINS[key]
    yield from cg.gpio_pin_expression({
        "number": config[key],
        "mode": "OUTPUT"
    });

@coroutine
def make_gpio_binary_output(key, config, gpio_var):
    type_, id_, *_ = PINS[key]
    output_var = cg.new_Pvariable(config[id_])
    cg.add(output_var.set_pin(gpio_var))   
    yield from cg.register_component(output_var, {})

@coroutine
def make_ledc_output(key, config, gpio_var):
    type_, id_, _, frequency, channel  = PINS[key]
    ledc_var = cg.new_Pvariable(config[id_], gpio_var)
    cg.add(ledc_var.set_frequency(frequency));
    cg.add(ledc_var.set_channel(channel));
    yield from cg.register_component(ledc_var, {})

def to_code(config):
    # Dirty little hack to make the ESPHome component loader inlcude
    # the code for the "gpio" platform for the "output" domain.
    # Loading specific platform components is not possible using
    # the AUTO_LOAD feature unfortunately.
    CORE.config["output"].append({ CONF_PLATFORM: "gpio" })

    hub_var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(hub_var, config)

    for key in PINS:
        type_ = PINS[key][0]
        gpio_var = yield make_gpio_pin(key, config)
        if type_ == LEDCOutput:
            pin_var = yield make_ledc_output(key, config, gpio_var)
        if type_ == GPIOBinaryOutput:
            pin_var = yield make_gpio_binary_output(key, config, gpio_var)
        setter = getattr(hub_var, "set_%s_pin" % key)
        cg.add(setter(pin_var))   

    trigger_pin = yield cg.gpio_pin_expression({
        "number": config[CONF_TRIGGER_PIN],
        "mode": "INPUT",
        "inverted": False
    })
    cg.add(hub_var.set_trigger_pin(trigger_pin))   

    for conf in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        yield automation.build_automation(trigger, [(float, "x")], conf)
