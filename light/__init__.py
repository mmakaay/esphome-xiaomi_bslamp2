import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome import automation
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE,
    CONF_OUTPUT_ID, CONF_TRIGGER_ID, CONF_ID
)
from .. import bslamp2_ns, CODEOWNERS, CONF_LIGHT_HAL_ID, LightHAL

AUTO_LOAD = ["xiaomi_bslamp2"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"

XiaomiBslamp2LightState = bslamp2_ns.class_("XiaomiBslamp2LightState", light.LightState)
XiaomiBslamp2LightOutput = bslamp2_ns.class_("XiaomiBslamp2LightOutput", light.LightOutput)
BrightnessTrigger = bslamp2_ns.class_("BrightnessTrigger", automation.Trigger.template())

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XiaomiBslamp2LightState),
        cv.GenerateID(CONF_LIGHT_HAL_ID): cv.use_id(LightHAL),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(XiaomiBslamp2LightOutput),
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
