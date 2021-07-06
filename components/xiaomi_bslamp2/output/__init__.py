import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID
from esphome import automation
from .. import (
    bslamp2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL, CONF_LEDS
)

AUTO_LOAD = ["xiaomi_bslamp2"]

XiaomiBslamp2FrontPanelOutput = bslamp2_ns.class_(
    "XiaomiBslamp2FrontPanelOutput", output.FloatOutput, cg.Component)
SetLEDsAction = bslamp2_ns.class_("SetLEDsAction", automation.Action)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XiaomiBslamp2FrontPanelOutput),
        cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.use_id(FrontPanelHAL),
    }
).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield output.register_output(var, config)

    front_panel_hal_var = yield cg.get_variable(config[CONF_FRONT_PANEL_HAL_ID])
    cg.add(var.set_parent(front_panel_hal_var))


def maybe_simple_leds_value(schema):
    def validator(value):
        if isinstance(value, dict):
            return schema(value)
        return schema({ "leds": value })
    return validator

@automation.register_action(
    "output.set_leds",
    SetLEDsAction,
    cv.Schema(
        maybe_simple_leds_value(cv.Schema({
            cv.GenerateID(CONF_ID): cv.use_id(XiaomiBslamp2FrontPanelOutput),
            cv.Required(CONF_LEDS): cv.templatable(cv.uint16_t),
        }))
    )
)
async def set_leds_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID]) 
    var = cg.new_Pvariable(action_id, template_arg, output_var)
    template_ = await cg.templatable(config[CONF_LEDS], args, cg.uint16)
    cg.add(var.set_leds(template_))
    return var
