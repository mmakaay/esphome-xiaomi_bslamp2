import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_FORCE_UPDATE
from .. import (
    bs2_ns, CODEOWNERS,
    CONF_FRONT_PANEL_HAL_ID, FrontPanelHAL
)

AUTO_LOAD = ["yeelight_bs2"]

YeelightBS2SliderSensor = bs2_ns.class_(
    "YeelightBS2SliderSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(YeelightBS2SliderSensor),
        cv.GenerateID(CONF_FRONT_PANEL_HAL_ID): cv.use_id(FrontPanelHAL),
        cv.Optional(CONF_FORCE_UPDATE, default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)

    front_panel_hal_var = yield cg.get_variable(config[CONF_FRONT_PANEL_HAL_ID])
    cg.add(var.set_parent(front_panel_hal_var))
