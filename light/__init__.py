import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.gpio.output as gpio_output
from esphome.components import light, gpio, ledc
from esphome.const import CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_OUTPUT_ID, CONF_TRIGGER_ID
from esphome import automation

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"

yeelight_ns = cg.esphome_ns.namespace("yeelight")
bs2_ns = yeelight_ns.namespace("bs2")
light_state = bs2_ns.class_("YeelightBS2LightState", cg.Nameable, cg.Component)
light_output = bs2_ns.class_("YeelightBS2LightOutput", light.LightOutput)

BrightnessTrigger = bs2_ns.class_("BrightnessTrigger", automation.Trigger.template())

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(light_state),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(light_output),
        cv.Required(CONF_RED): cv.use_id(ledc),
        cv.Required(CONF_GREEN): cv.use_id(ledc),
        cv.Required(CONF_BLUE): cv.use_id(ledc),
        cv.Required(CONF_WHITE): cv.use_id(ledc),
        cv.Required(CONF_WHITE): cv.use_id(ledc),
        cv.Required(CONF_MASTER1): cv.use_id(gpio_output.GPIOBinaryOutput),
        cv.Required(CONF_MASTER2): cv.use_id(gpio_output.GPIOBinaryOutput),

        cv.Optional(CONF_ON_BRIGHTNESS): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(BrightnessTrigger),
            }
        ),
    }
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)

    led_white = yield cg.get_variable(config[CONF_WHITE])
    cg.add(var.set_white_output(led_white))

    led_red = yield cg.get_variable(config[CONF_RED])
    cg.add(var.set_red_output(led_red))

    led_green = yield cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_green_output(led_green))

    led_blue = yield cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_blue_output(led_blue))

    master1 = yield cg.get_variable(config[CONF_MASTER1])
    cg.add(var.set_master1_output(master1))

    master2 = yield cg.get_variable(config[CONF_MASTER2])
    cg.add(var.set_master2_output(master2))

    for conf in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        yield automation.build_automation(trigger, [(float, "x")], conf)
