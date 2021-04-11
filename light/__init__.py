import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.gpio.output as gpio_output
from esphome.components import light, gpio, ledc
from esphome.const import CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_OUTPUT_ID, CONF_TRIGGER_ID
from esphome import automation
from .. import bs2_ns, CODEOWNERS, CONF_HUB_ID, YeelightBS2Hub

AUTO_LOAD = ["yeelight_bs2"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"

light_state = bs2_ns.class_("YeelightBS2LightState", light.LightState)
light_output = bs2_ns.class_("YeelightBS2LightOutput", light.LightOutput)

BrightnessTrigger = bs2_ns.class_("BrightnessTrigger", automation.Trigger.template())

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(light_state),
        cv.GenerateID(CONF_HUB_ID): cv.use_id(YeelightBS2Hub),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(light_output),
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

    hub_var = yield cg.get_variable(config[CONF_HUB_ID])
    cg.add(var.set_hub(hub_var))

    for conf in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        yield automation.build_automation(trigger, [(float, "x")], conf)
