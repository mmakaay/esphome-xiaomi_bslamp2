import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID
from .. import (
    bslamp2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL
)

AUTO_LOAD = ["xiaomi_bslamp2"]

XiaomiBslamp2FrontPanelLight = bslamp2_ns.class_(
    "XiaomiBslamp2FrontPanelLight", output.FloatOutput, cg.Component)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XiaomiBslamp2FrontPanelLight),
        cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.use_id(FrontPanelHAL),
    }
).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield output.register_output(var, config)

    front_panel_hal_var = yield cg.get_variable(config[CONF_FRONT_PANEL_HAL_ID])
    cg.add(var.set_parent(front_panel_hal_var))
