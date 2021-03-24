import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output, gpio
from esphome.const import CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_OUTPUT_ID


CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"

rgbww_ns = cg.esphome_ns.namespace("rgbww")
YeelightBS2LightOutput = rgbww_ns.class_("YeelightBS2LightOutput", light.LightOutput)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(YeelightBS2LightOutput),
        cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
        cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
        cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_MASTER1): cv.use_id(gpio.output.GPIOBinaryOutput),
        cv.Required(CONF_MASTER2): cv.use_id(gpio.output.GPIOBinaryOutput),
    }
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)

    red = yield cg.get_variable(config[CONF_RED])
    cg.add(var.set_red(red))
    green = yield cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_green(green))
    blue = yield cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_blue(blue))
    white = yield cg.get_variable(config[CONF_WHITE])
    cg.add(var.set_white(white))
    master1 = yield cg.get_variable(config[CONF_MASTER1])
    cg.add(var.set_master1(master1))
    master2 = yield cg.get_variable(config[CONF_MASTER2])
    cg.add(var.set_master2(master2))
