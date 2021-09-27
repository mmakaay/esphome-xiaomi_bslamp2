import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_LEVEL
from esphome import automation
from .. import (
    bslamp2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL, FRONT_PANEL_LED_OPTIONS
)

CONF_LEDS = "leds"

DEPENDENCIES = ["xiaomi_bslamp2"]

XiaomiBslamp2FrontPanelOutput = bslamp2_ns.class_(
    "XiaomiBslamp2FrontPanelOutput", output.FloatOutput, cg.Component)
SetLEDsAction = bslamp2_ns.class_("SetLEDsAction", automation.Action)
SetLevelAction = bslamp2_ns.class_("SetLevelAction", automation.Action)
UpdateLEDsAction = bslamp2_ns.class_("UpdateLEDsAction", automation.Action)

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

def maybe_simple_level_value(schema):
    def validator(value):
        if isinstance(value, dict):
            return schema(value)
        return schema({ "level": value })
    return validator

def maybe_simple_leds_value(schema):
    def validator(value):
        if isinstance(value, dict):
            return schema(value)
        return schema({ "leds": value })
    return validator

FRONT_PANEL_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(XiaomiBslamp2FrontPanelOutput),
})

FRONT_PANEL_LEVEL_SCHEMA = cv.Schema(
    maybe_simple_level_value(FRONT_PANEL_SCHEMA.extend(
        {
            cv.Required(CONF_LEVEL): cv.templatable(cv.percentage),
        }
    ))
)

FRONT_PANEL_LED_SCHEMA = cv.Schema(
    maybe_simple_leds_value(FRONT_PANEL_SCHEMA.extend(
        {
            cv.Required(CONF_LEDS): cv.ensure_list(cv.enum(FRONT_PANEL_LED_OPTIONS, upper=True)),
        }
    ))
)

@automation.register_action("front_panel.set_level", SetLevelAction, FRONT_PANEL_LEVEL_SCHEMA)
async def set_level_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID])
    action_var = cg.new_Pvariable(action_id, template_arg, output_var)
    template_ = await cg.templatable(config[CONF_LEVEL], args, float)
    cg.add(action_var.set_level(template_))
    return action_var

@automation.register_action("front_panel.set_leds", SetLEDsAction, FRONT_PANEL_LED_SCHEMA)
async def set_leds_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, output_var)
    bits = (
        [FRONT_PANEL_LED_OPTIONS['NONE']] +
        [FRONT_PANEL_LED_OPTIONS[led] for led in config[CONF_LEDS]]
    )
    value = cg.RawExpression("|".join(map(str, bits)))
    cg.add(action_var.set_mode(2))
    cg.add(action_var.set_leds(value))
    return action_var

@automation.register_action("front_panel.turn_on_leds", SetLEDsAction, FRONT_PANEL_LED_SCHEMA)
async def turn_on_leds_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, output_var)
    bits = (
        [FRONT_PANEL_LED_OPTIONS['NONE']] +
        [FRONT_PANEL_LED_OPTIONS[led] for led in config[CONF_LEDS]]
    )
    value = cg.RawExpression("|".join(map(str, bits)))
    cg.add(action_var.set_mode(1))
    cg.add(action_var.set_leds(value))
    return action_var

@automation.register_action("front_panel.turn_off_leds", SetLEDsAction, FRONT_PANEL_LED_SCHEMA)
async def turn_off_leds_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, output_var)
    bits = (
        [FRONT_PANEL_LED_OPTIONS['NONE']] +
        [FRONT_PANEL_LED_OPTIONS[led] for led in config[CONF_LEDS]]
    )
    value = cg.RawExpression("|".join(map(str, bits)))
    cg.add(action_var.set_mode(0))
    cg.add(action_var.set_leds(value))
    return action_var

@automation.register_action("front_panel.update_leds", UpdateLEDsAction, FRONT_PANEL_SCHEMA)
async def update_leds_to_code(config, action_id, template_arg, args):
    output_var = await cg.get_variable(config[CONF_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, output_var)
    return action_var
