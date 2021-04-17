import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome import automation
from esphome.core import coroutine
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_COLOR_TEMPERATURE,
    CONF_OUTPUT_ID, CONF_TRIGGER_ID, CONF_ID,
    CONF_TRANSITION_LENGTH, CONF_BRIGHTNESS, CONF_EFFECT
)
from .. import bslamp2_ns, CODEOWNERS, CONF_LIGHT_HAL_ID, LightHAL

AUTO_LOAD = ["xiaomi_bslamp2"]

CONF_MASTER1 = "master1"
CONF_MASTER2 = "master2"
CONF_ON_BRIGHTNESS = "on_brightness"
CONF_PRESET_ID = "preset_id"
CONF_PRESETS_ID = "presets_id"
CONF_PRESETS = "presets"
CONF_NEXT = "next"
CONF_GROUP = "group"
CONF_PRESET = "preset"

XiaomiBslamp2LightState = bslamp2_ns.class_("XiaomiBslamp2LightState", light.LightState)
XiaomiBslamp2LightOutput = bslamp2_ns.class_("XiaomiBslamp2LightOutput", light.LightOutput)
PresetsContainer = bslamp2_ns.class_("PresetsContainer", cg.Component)
Preset = bslamp2_ns.class_("Preset", cg.Component)
BrightnessTrigger = bslamp2_ns.class_("BrightnessTrigger", automation.Trigger.template())
ActivatePresetAction = bslamp2_ns.class_("ActivatePresetAction", automation.Action)

PRESETS_SCHEMA = cv.Schema({
    str.lower: cv.Schema({
        str.lower: light.automation.LIGHT_TURN_ON_ACTION_SCHEMA
    })
})

PRESET_SCHEMA_BASE = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(XiaomiBslamp2LightState),
        cv.GenerateID(CONF_PRESET_ID): cv.declare_id(Preset),
    }
)

PRESET_SCHEMA = cv.Any(
    PRESET_SCHEMA_BASE.extend({
        cv.Required(CONF_EFFECT): cv.string
    }),
    PRESET_SCHEMA_BASE.extend({
        cv.Required(CONF_COLOR_TEMPERATURE): cv.color_temperature,
        cv.Optional(CONF_BRIGHTNESS): cv.percentage,
        cv.Optional(CONF_TRANSITION_LENGTH): cv.positive_time_period_milliseconds,
    }),
    PRESET_SCHEMA_BASE.extend({
        cv.Required(CONF_RED): cv.percentage,
        cv.Required(CONF_GREEN): cv.percentage,
        cv.Required(CONF_BLUE): cv.percentage,
        cv.Optional(CONF_BRIGHTNESS): cv.percentage,
        cv.Optional(CONF_TRANSITION_LENGTH): cv.positive_time_period_milliseconds,
    }),
)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(XiaomiBslamp2LightState),
        cv.GenerateID(CONF_LIGHT_HAL_ID): cv.use_id(LightHAL),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(XiaomiBslamp2LightOutput),
        cv.Optional(CONF_ON_BRIGHTNESS): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(BrightnessTrigger),
            }
        ),
        cv.GenerateID(CONF_PRESETS_ID): cv.declare_id(PresetsContainer),
        cv.Optional(CONF_PRESETS): cv.Schema({
            str.lower: cv.Schema({
                str.lower: PRESET_SCHEMA
            })
        }),
    }
)

def is_preset_group(value):
    return value

def is_preset(value):
    return value

def maybe_simple_preset_action(schema):
    def validator(value):
        if isinstance(value, dict):
            return schema(value)
        value = value.lower()
        conf = {}
        if value == "next_group":
            conf[CONF_NEXT] = CONF_GROUP
        elif value == "next_preset":
            conf[CONF_NEXT] = CONF_PRESET
        elif "." not in value:
            conf[CONF_GROUP] = value
        else:
            group, preset = value.split(".", 2)
            conf[CONF_GROUP] = group
            conf[CONF_PRESET] = preset
        return schema(conf)

    return validator

@automation.register_action(
    "preset.activate",
    ActivatePresetAction,
    cv.Schema(
        maybe_simple_preset_action(cv.Any(
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
def preset_to_code(config, preset_group, preset_name):
    light_var = yield cg.get_variable(config[CONF_ID])
    preset_var = cg.new_Pvariable(
        config[CONF_PRESET_ID], light_var, preset_group, preset_name)
    if CONF_TRANSITION_LENGTH in config:
        cg.add(preset_var.set_transition_length(config[CONF_TRANSITION_LENGTH]))
    if CONF_BRIGHTNESS in config:
        cg.add(preset_var.set_brightness(config[CONF_BRIGHTNESS]))
    if CONF_RED in config:
        cg.add(preset_var.set_red(config[CONF_RED]))
    if CONF_GREEN in config:
        cg.add(preset_var.set_green(config[CONF_GREEN]))
    if CONF_BLUE in config:
        cg.add(preset_var.set_blue(config[CONF_BLUE]))
    if CONF_COLOR_TEMPERATURE in config:
        cg.add(preset_var.set_color_temperature(config[CONF_COLOR_TEMPERATURE]))
    if CONF_EFFECT in config:
        cg.add(preset_var.set_effect(config[CONF_EFFECT]))
    else:
        cg.add(preset_var.set_effect("None"))
    yield cg.register_component(preset_var, config)

@coroutine
def presets_to_code(config):
    presets_var = cg.new_Pvariable(config[CONF_PRESETS_ID])
    yield cg.register_component(presets_var, config)

    for preset_group, presets in config.get(CONF_PRESETS, {}).items():
        for preset_name, preset_config in presets.items():
            preset = yield preset_to_code(preset_config, preset_group, preset_name)
            cg.add(presets_var.add_preset(preset))

def to_code(config):
    yield light_output_to_code(config)
    yield on_brightness_to_code(config)
    yield presets_to_code(config)
