import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import output
from esphome.components.ledc.output import LEDCOutput, validate_frequency
from esphome.core import coroutine
from esphome.core import CORE
from esphome.config import get_platform
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
    CONF_CHANNEL
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

AUTO_LOAD = ["ledc"]

yeelight_ns = cg.esphome_ns.namespace("yeelight")
bs2_ns = yeelight_ns.namespace("bs2")
YeelightBS2Hub = bs2_ns.class_("YeelightBS2Hub", cg.Component)

LEDC_PINS = {
    # Config key     ID             PIN       FREQ   CH
    CONF_RED     : ( CONF_RED_ID,   "GPIO13", 3000,  0 ),
    CONF_GREEN   : ( CONF_GREEN_ID, "GPIO14", 3000,  1 ),
    CONF_BLUE    : ( CONF_BLUE_ID,  "GPIO5",  3000,  2 ),
    CONF_WHITE   : ( CONF_WHITE_ID, "GPIO12", 10000, 4 ),
}

def make_config_schema():
    schema = cv.COMPONENT_SCHEMA.extend({
        cv.GenerateID(): cv.declare_id(YeelightBS2Hub),
        cv.Optional(CONF_TRIGGER_PIN, default="GPIO16"): cv.All(
            pins.validate_gpio_pin,
            pins.validate_has_interrupt
        ),
    })

    for key, pin_config in LEDC_PINS.items():
        id_, pin, *_ = pin_config
        schema = schema.extend({
            cv.GenerateID(id_): cv.declare_id(LEDCOutput),
            cv.Optional(key, default=pin): pins.validate_gpio_pin
        })

    return schema;


CONFIG_SCHEMA = make_config_schema()

@coroutine
def make_ledc_pin(key, config):
    id_, _, frequency, channel = LEDC_PINS[key]
    gpio_var = yield cg.gpio_pin_expression({
        "number": config[key],
        "mode": "OUTPUT"
    });
    ledc_var = cg.new_Pvariable(config[id_], gpio_var)
    cg.add(ledc_var.set_frequency(frequency));
    cg.add(ledc_var.set_channel(channel));
    yield from cg.register_component(ledc_var, {}) # TODO last arg?

def to_code(config):
    hub_var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(hub_var, config)

    for key in LEDC_PINS:
        ledc_pin = yield make_ledc_pin(key, config)
        setter = getattr(hub_var, "set_%s_pin" % key)
        cg.add(setter(ledc_pin))   

    trigger_pin = yield cg.gpio_pin_expression({
        "number": config[CONF_TRIGGER_PIN],
        "mode": "INPUT",
        "inverted": False
    })
    cg.add(hub_var.set_trigger_pin(trigger_pin))   
# 
#     led_white = yield cg.get_variable(config[CONF_WHITE])
#     cg.add(var.set_white_output(led_white))
# 
#     led_red = yield cg.get_variable(config[CONF_RED])
#     cg.add(var.set_red_output(led_red))
# 
#     led_green = yield cg.get_variable(config[CONF_GREEN])
#     cg.add(var.set_green_output(led_green))
# 
#     led_blue = yield cg.get_variable(config[CONF_BLUE])
#     cg.add(var.set_blue_output(led_blue))
# 
#     master1 = yield cg.get_variable(config[CONF_MASTER1])
#     cg.add(var.set_master1_output(master1))
# 
#     master2 = yield cg.get_variable(config[CONF_MASTER2])
#     cg.add(var.set_master2_output(master2))
# 
#     for conf in config.get(CONF_ON_BRIGHTNESS, []):
#         trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
#         yield automation.build_automation(trigger, [(float, "x")], conf)
