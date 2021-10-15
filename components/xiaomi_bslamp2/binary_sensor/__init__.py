import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_FOR
from .. import (
    bslamp2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL
)

DEPENDENCIES = ["xiaomi_bslamp2"]

CONF_PART = "part"

# The identifier values match the bit values of the events as defined 
# in ../front_panel_hal.h.
PARTS = {
    "POWER_BUTTON" : 0b001 << 1,
    "POWER"        : 0b001 << 1,
    "COLOR_BUTTON" : 0b010 << 1,
    "COLOR"        : 0b010 << 1,
    "SLIDER"       : 0b100 << 1,
}

XiaomiBslamp2TouchBinarySensor = bslamp2_ns.class_(
    "XiaomiBslamp2TouchBinarySensor", binary_sensor.BinarySensor, cg.Component)

def validate_for(value):
    value = cv.string(value)
    return cv.enum(PARTS, upper=True, space='_')(value)

def validate_binary_sensor(conf):
    if CONF_PART in conf and CONF_FOR in conf:
        raise cv.Invalid("Specify only one of [part] or [for]")
    if CONF_PART in conf and not CONF_FOR in conf:
        # Backward compatibility.
        conf[CONF_FOR] = conf[CONF_PART]
    if CONF_FOR not in conf:
        raise cv.Invalid("'for' is a required option for [binary_sensor.xiaomi_bslamp2]")
    return conf

CONFIG_SCHEMA = cv.All(
    binary_sensor.BINARY_SENSOR_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(XiaomiBslamp2TouchBinarySensor),
            cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.use_id(FrontPanelHAL),
            # This option is not advertised in the documentation. It must be
            # considered deprecated. I'm not announcing it as such yet. Not sure
            # if it's useful to do so.
            cv.Optional(CONF_PART): validate_for,
            cv.Optional(CONF_FOR): validate_for,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_binary_sensor,
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield binary_sensor.register_binary_sensor(var, config)

    front_panel_hal_var = yield cg.get_variable(config[CONF_FRONT_PANEL_HAL_ID])
    cg.add(var.set_parent(front_panel_hal_var))
    cg.add(var.set_for(config[CONF_FOR]))
