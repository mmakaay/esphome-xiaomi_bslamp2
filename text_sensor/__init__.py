import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_OUTPUT_ID
from .. import bslamp2_ns, CODEOWNERS
from ..light import XiaomiBslamp2LightOutput

DEPENDENCIES = ["xiaomi_bslamp2"]

CONF_LIGHT_ID = "light_id"

XiaomiBslamp2LightModeTextSensor = bslamp2_ns.class_(
    "XiaomiBslamp2LightModeTextSensor", text_sensor.TextSensor, cg.Component
)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XiaomiBslamp2LightModeTextSensor),
        cv.GenerateID(CONF_OUTPUT_ID): cv.use_id(XiaomiBslamp2LightOutput),
    }
).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield text_sensor.register_text_sensor(var, config)

    parent_var = yield cg.get_variable(config[CONF_OUTPUT_ID])
    cg.add(var.set_parent(parent_var))
