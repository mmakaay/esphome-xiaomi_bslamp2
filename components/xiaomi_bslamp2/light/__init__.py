import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome import automation
from esphome.core import Lambda
from esphome.const import (
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_COLOR_TEMPERATURE,
    CONF_STATE, CONF_OUTPUT_ID, CONF_TRIGGER_ID, CONF_ID,
    CONF_TRANSITION_LENGTH, CONF_BRIGHTNESS, CONF_EFFECT, CONF_FLASH_LENGTH
)
from .. import bslamp2_ns, CODEOWNERS, CONF_LIGHT_HAL_ID, LightHAL

DEPENDENCIES = ["xiaomi_bslamp2"]

CONF_ON_BRIGHTNESS = "on_brightness"
CONF_PRESET_ID = "preset_id"
CONF_PRESETS_ID = "presets_id"
CONF_PRESET = "preset"
CONF_PRESETS = "presets"
CONF_NEXT = "next"
CONF_GROUP = "group"

MIRED_MIN = 153
MIRED_MAX = 588

XiaomiBslamp2LightState = bslamp2_ns.class_("XiaomiBslamp2LightState", light.LightState)
XiaomiBslamp2LightOutput = bslamp2_ns.class_("XiaomiBslamp2LightOutput", light.LightOutput)
PresetsContainer = bslamp2_ns.class_("PresetsContainer", cg.Component)
Preset = bslamp2_ns.class_("Preset", cg.Component)
BrightnessTrigger = bslamp2_ns.class_("BrightnessTrigger", automation.Trigger.template())
ActivatePresetAction = bslamp2_ns.class_("ActivatePresetAction", automation.Action)
DiscoAction = bslamp2_ns.class_("DiscoAction", automation.Action)

PRESETS_SCHEMA = cv.Schema({
    str.lower: cv.Schema({
        str.lower: light.automation.LIGHT_TURN_ON_ACTION_SCHEMA
    })
})

def validate_preset(config):
    has_rgb = CONF_RED in config or CONF_GREEN in config or CONF_BLUE in config
    has_white = CONF_COLOR_TEMPERATURE in config
    has_effect = CONF_EFFECT in config

    # Check mutual exclusivity of preset options.
    if (has_rgb + has_white + has_effect) > 1:
        raise cv.Invalid("Use only one of RGB light, white (color temperature) light or an effect")

    # Check the color temperature value range.
    if has_white:
        if config[CONF_COLOR_TEMPERATURE] < MIRED_MIN or config[CONF_COLOR_TEMPERATURE] > MIRED_MAX:
            raise cv.Invalid(f"The color temperature must be in the range {MIRED_MIN} - {MIRED_MAX}")

    # When defining an RGB color, it is allowed to omit RGB components that have value 0.
    if has_rgb:
        if CONF_RED not in config:
            config[CONF_RED] = 0
        if CONF_GREEN not in config:
            config[CONF_GREEN] = 0
        if CONF_BLUE not in config:
            config[CONF_BLUE] = 0

    return config

PRESET_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.use_id(XiaomiBslamp2LightState),
            cv.GenerateID(CONF_PRESET_ID): cv.declare_id(Preset),
            cv.Optional(CONF_EFFECT): cv.string,
            cv.Optional(CONF_COLOR_TEMPERATURE): cv.color_temperature,
            cv.Optional(CONF_RED): cv.percentage,
            cv.Optional(CONF_GREEN): cv.percentage,
            cv.Optional(CONF_BLUE): cv.percentage,
            cv.Optional(CONF_BRIGHTNESS): cv.percentage,
            cv.Optional(CONF_TRANSITION_LENGTH): cv.positive_time_period_milliseconds,
        }
    ),
    validate_preset
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
        config = {}
        if value == "next_group":
            config[CONF_NEXT] = CONF_GROUP
        elif value == "next_preset":
            config[CONF_NEXT] = CONF_PRESET
        elif "." not in value:
            config[CONF_GROUP] = value
        else:
            group, preset = value.split(".", 2)
            config[CONF_GROUP] = group
            config[CONF_PRESET] = preset
        return schema(config)

    return validator

@automation.register_action(
    "light.disco_on", DiscoAction, light.automation.LIGHT_TURN_ON_ACTION_SCHEMA 
)
def disco_action_on_to_code(config, action_id, template_arg, args):
    light_var = yield cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, light_var)

    if CONF_STATE in config:
        template_ = yield cg.templatable(config[CONF_STATE], args, bool)
        cg.add(var.set_state(template_))
    if CONF_TRANSITION_LENGTH in config:
        template_ = yield cg.templatable(
            config[CONF_TRANSITION_LENGTH], args, cg.uint32
        )
        cg.add(var.set_transition_length(template_))
    if CONF_FLASH_LENGTH in config:
        template_ = yield cg.templatable(config[CONF_FLASH_LENGTH], args, cg.uint32)
        cg.add(var.set_flash_length(template_))
    if CONF_BRIGHTNESS in config:
        template_ = yield cg.templatable(config[CONF_BRIGHTNESS], args, float)
        cg.add(var.set_brightness(template_))
    if CONF_RED in config:
        template_ = yield cg.templatable(config[CONF_RED], args, float)
        cg.add(var.set_red(template_))
    if CONF_GREEN in config:
        template_ = yield cg.templatable(config[CONF_GREEN], args, float)
        cg.add(var.set_green(template_))
    if CONF_BLUE in config:
        template_ = yield cg.templatable(config[CONF_BLUE], args, float)
        cg.add(var.set_blue(template_))
    if CONF_COLOR_TEMPERATURE in config:
        template_ = yield cg.templatable(config[CONF_COLOR_TEMPERATURE], args, float)
        cg.add(var.set_color_temperature(template_))
    if CONF_EFFECT in config:
        template_ = yield cg.templatable(config[CONF_EFFECT], args, cg.std_string)
        cg.add(var.set_effect(template_))
    yield var

@automation.register_action(
    "light.disco_off", DiscoAction, light.automation.LIGHT_TURN_OFF_ACTION_SCHEMA 
)
def disco_action_off_to_code(config, action_id, template_arg, args):
    light_var = yield cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, light_var)
    cg.add(var.set_disco_state(False))
    yield var

USED_PRESETS = []

def register_preset_action(value):
    if "group" in value and not isinstance(value["group"], Lambda):
        if "preset" in value and not isinstance(value["preset"], Lambda):
            preset_data = [value['group'], value['preset']]
        else:
            preset_data = [value["group"], None]
        USED_PRESETS.append(preset_data)
    return value

@automation.register_action(
    "preset.activate",
    ActivatePresetAction,
    cv.All(
        maybe_simple_preset_action(cv.Any(
            cv.Schema({
                cv.GenerateID(CONF_PRESETS_ID): cv.use_id(PresetsContainer),
                cv.Required(CONF_GROUP): cv.templatable(cv.string),
                cv.Optional(CONF_PRESET): cv.templatable(cv.string)
            }),
            cv.Schema({
                cv.GenerateID(CONF_PRESETS_ID): cv.use_id(PresetsContainer),
                cv.Required(CONF_NEXT): cv.one_of(CONF_GROUP, CONF_PRESET, lower=True)
            })
        )),
        register_preset_action
    ),
)
def preset_activate_to_code(config, action_id, template_arg, args):
    presets_var = yield cg.get_variable(config[CONF_PRESETS_ID]) 
    action_var = cg.new_Pvariable(action_id, template_arg, presets_var)
    if CONF_NEXT in config:
        cg.add(action_var.set_operation(f"next_{config[CONF_NEXT]}"))
    elif CONF_PRESET in config:
        cg.add(action_var.set_operation("activate_preset"))
        group_template_ = yield cg.templatable(config[CONF_GROUP], args, cg.std_string)
        cg.add(action_var.set_group(group_template_))
        preset_template_ = yield cg.templatable(config[CONF_PRESET], args, cg.std_string)
        cg.add(action_var.set_preset(preset_template_))
    else:
        cg.add(action_var.set_operation("activate_group"))
        group_template_ = yield cg.templatable(config[CONF_GROUP], args, cg.std_string)
        cg.add(action_var.set_group(group_template_))
    yield action_var

async def light_output_to_code(config):
    light_output_var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(light_output_var, config)
    light_hal_var = await cg.get_variable(config[CONF_LIGHT_HAL_ID])
    cg.add(light_output_var.set_parent(light_hal_var))

async def on_brightness_to_code(config):
    light_output_var = await cg.get_variable(config[CONF_OUTPUT_ID])
    for config in config.get(CONF_ON_BRIGHTNESS, []):
        trigger = cg.new_Pvariable(config[CONF_TRIGGER_ID], light_output_var)
        await automation.build_automation(trigger, [(float, "x")], config)

async def preset_to_code(config, preset_group, preset_name):
    light_var = await cg.get_variable(config[CONF_ID])
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
    return await cg.register_component(preset_var, config)

async def presets_to_code(config):
    presets_var = cg.new_Pvariable(config[CONF_PRESETS_ID])
    await cg.register_component(presets_var, config)

    for preset_group, presets in config.get(CONF_PRESETS, {}).items():
        for preset_name, preset_config in presets.items():
            preset = await preset_to_code(preset_config, preset_group, preset_name)
            cg.add(presets_var.add_preset(preset))

async def to_code(config):
    await light_output_to_code(config)
    await on_brightness_to_code(config)
    await presets_to_code(config)

def validate(config):
    valid_presets = config.get(CONF_PRESETS, {});
    for group, preset in USED_PRESETS:
        if group not in valid_presets:
            raise cv.Invalid(f"Invalid light preset group '{group}' used")
        if preset is not None and preset not in valid_presets[group]:
            raise cv.Invalid(f"Invalid light preset '{group}.{preset}' used")
    return config

FINAL_VALIDATE_SCHEMA = cv.Schema(validate);

