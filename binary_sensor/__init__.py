import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
#from esphome import automation
from esphome.const import CONF_DEVICE_CLASS, CONF_ID
from .. import (
    bs2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL
)

AUTO_LOAD = ["yeelight_bs2"]

CONF_PART = "part"

PARTS = {
    "ANY"          : 0,
    "POWER_BUTTON" : 1,
    "COLOR_BUTTON" : 2,
    "SLIDER"       : 3.
}

def validate_part(value):
    value = cv.string(value)
    return cv.enum(PARTS, upper=True)(value)

YeelightBS2Button = bs2_ns.class_("YeelightBS2Button", binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(YeelightBS2Button),
        cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.use_id(FrontPanelHAL),
        cv.Optional(CONF_PART, default="ANY"): validate_part,
    }
).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield binary_sensor.register_binary_sensor(var, config)

    front_panel_hal_var = yield cg.get_variable(config[CONF_FRONT_PANEL_HAL_ID])
    cg.add(var.set_front_panel_hal(front_panel_hal_var))
    cg.add(var.set_part(config[CONF_PART]))
