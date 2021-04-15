import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome import automation
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE,
    CONF_OUTPUT_ID, CONF_TRIGGER_ID, CONF_ID
)
from .. import bs2_ns, CODEOWNERS, CONF_LIGHT_HAL_ID, LightHAL

AUTO_LOAD = ["yeelight_bs2"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"

YeelightBS2LightState = bs2_ns.class_("YeelightBS2LightState", light.LightState)
YeelightBS2LightOutput = bs2_ns.class_("YeelightBS2LightOutput", light.LightOutput)
BrightnessTrigger = bs2_ns.class_("BrightnessTrigger", automation.Trigger.template())
LightModeTrigger = bs2_ns.class_("LightModeTrigger", automation.Trigger.template())

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(YeelightBS2LightState),
        cv.GenerateID(CONF_LIGHT_HAL_ID): cv.use_id(LightHAL),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(YeelightBS2LightOutput),
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

    light_hal_var = yield cg.get_variable(config[CONF_LIGHT_HAL_ID])
    cg.add(var.set_parent(light_hal_var))

    for conf in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        yield automation.build_automation(trigger, [(float, "x")], conf)
