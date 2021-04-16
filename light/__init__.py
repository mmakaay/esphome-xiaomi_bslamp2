import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome import automation
from esphome.core import coroutine
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_COLOR_TEMPERATURE,
    CONF_OUTPUT_ID, CONF_TRIGGER_ID, CONF_ID,
)
from .. import bslamp2_ns, CODEOWNERS, CONF_LIGHT_HAL_ID, LightHAL

AUTO_LOAD = ["xiaomi_bslamp2"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"
CONF_PRESETS_ID = "presets_id"
CONF_PRESETS = "presets"
CONF_NEXT = "next"
CONF_GROUP = "group"
CONF_PRESET = "preset"

XiaomiBslamp2LightState = bslamp2_ns.class_("XiaomiBslamp2LightState", light.LightState)
XiaomiBslamp2LightOutput = bslamp2_ns.class_("XiaomiBslamp2LightOutput", light.LightOutput)
PresetsContainer = bslamp2_ns.class_("PresetsContainer", cg.Component)
BrightnessTrigger = bslamp2_ns.class_("BrightnessTrigger", automation.Trigger.template())
ActivatePresetAction = bslamp2_ns.class_("ActivatePresetAction", automation.Action)

PRESETS_SCHEMA = cv.Schema({
    str.lower: cv.Schema({
        str.lower: cv.Any(
            cv.Schema({
                cv.Optional(CONF_RED, default=0): cv.percentage,
                cv.Optional(CONF_GREEN, default=0): cv.percentage,
                cv.Optional(CONF_BLUE, default=0): cv.percentage,
            }),
            cv.Schema({
                cv.Required(CONF_COLOR_TEMPERATURE): cv.int_range(min=153, max=588),
            }),
        )
    })
})

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
        cv.GenerateID(CONF_PRESETS_ID): cv.declare_id(PresetsContainer),
        cv.Optional(CONF_PRESETS): PRESETS_SCHEMA,
    }
)

def is_preset_group(value):
    return value

def is_preset(value):
    return value

@automation.register_action(
    "preset.activate",
    ActivatePresetAction,
    cv.Schema(cv.Any(
        cv.Schema({
            cv.GenerateID(CONF_PRESETS_ID): cv.use_id(PresetsContainer),
            cv.Required(CONF_GROUP): is_preset_group,
            cv.Optional(CONF_PRESET): is_preset
        }),
        cv.Schema({
            cv.GenerateID(CONF_PRESETS_ID): cv.use_id(PresetsContainer),
            cv.Required(CONF_NEXT): cv.one_of(CONF_GROUP, CONF_PRESET, lower=True)
        })
         
    ))
)
def preset_activate_to_code(config, action_id, template_arg, args):
    presets_var = yield cg.get_variable(config[CONF_PRESETS_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, presets_var)
    if CONF_NEXT in config:
        cg.add(action_var.set_operation(f"next_{config[CONF_NEXT]}"))
    elif CONF_PRESET in config:
        cg.add(action_var.set_operation("activate_preset"))
        cg.add(action_var.set_group(config[CONF_GROUP]))
        cg.add(action_var.set_preset(config[CONF_PRESET]))
    else:
        cg.add(action_var.set_operation("activate_group"))
        cg.add(action_var.set_group(config[CONF_GROUP]))
    yield action_var

@coroutine
def light_output_to_code(config):
    light_output_var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(light_output_var, config)
    light_hal_var = yield cg.get_variable(config[CONF_LIGHT_HAL_ID])
    cg.add(light_output_var.set_parent(light_hal_var))

@coroutine
def on_brightness_to_code(config):
    light_output_var = yield cg.get_variable(config[CONF_OUTPUT_ID])
    for conf in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], light_output_var)
        yield automation.build_automation(trigger, [(float, "x")], conf)

@coroutine
def presets_to_code(config):
    light_state_var = yield cg.get_variable(config[CONF_ID])
    presets_var = cg.new_Pvariable(config[CONF_PRESETS_ID], light_state_var)
    yield cg.register_component(presets_var, config)

    for preset_group, presets in config.get(CONF_PRESETS, {}).items():
        for name, preset in presets.items():
            if CONF_COLOR_TEMPERATURE in preset:
                cg.add(presets_var.add(
                    preset_group, name, preset[CONF_COLOR_TEMPERATURE]))
            else:
                cg.add(presets_var.add(
                    preset_group, name, preset[CONF_RED], preset[CONF_GREEN], preset[CONF_BLUE]))

def to_code(config):
    yield light_output_to_code(config)
    yield on_brightness_to_code(config)
    yield presets_to_code(config)
