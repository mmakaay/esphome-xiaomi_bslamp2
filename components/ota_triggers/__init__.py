from esphome.cpp_generator import RawExpression
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.ota import ota_ns, OTAComponent
from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@mmakaay"]
DEPENDENCIES = ["ota"]

CONF_ON_BEGIN = "on_begin"
CONF_ON_PROGRESS = "on_progress"
CONF_ON_END = "on_end"
CONF_ON_ERROR = "on_error"
CONF_ON_BEGIN_TRIGGER_ID = "on_begin_trigger_id"
CONF_ON_PROGRESS_TRIGGER_ID = "on_progress_trigger_id"
CONF_ON_END_TRIGGER_ID = "on_end_trigger_id"
CONF_ON_ERROR_TRIGGER_ID = "on_error_trigger_id"

OTAStartTrigger = ota_ns.class_("OTAStartTrigger", automation.Trigger.template())
OTAProgressTrigger = ota_ns.class_("OTAProgressTrigger", automation.Trigger.template())
OTAEndTrigger = ota_ns.class_("OTAEndTrigger", automation.Trigger.template())
OTAErrorTrigger = ota_ns.class_("OTAErrorTrigger", automation.Trigger.template())

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(OTAComponent),
        cv.Optional(CONF_ON_BEGIN): automation.validate_automation(
            {
                cv.GenerateID(CONF_ON_BEGIN_TRIGGER_ID): cv.declare_id(OTAStartTrigger),
            }
        ),
        cv.Optional(CONF_ON_ERROR): automation.validate_automation(
            {
                cv.GenerateID(CONF_ON_ERROR_TRIGGER_ID): cv.declare_id(OTAErrorTrigger),
            }
        ),
        cv.Optional(CONF_ON_PROGRESS): automation.validate_automation(
            {
                cv.GenerateID(CONF_ON_PROGRESS_TRIGGER_ID): cv.declare_id(
                    OTAProgressTrigger
                ),
            }
        ),
        cv.Optional(CONF_ON_END): automation.validate_automation(
            {
                cv.GenerateID(CONF_ON_END_TRIGGER_ID): cv.declare_id(OTAEndTrigger),
            }
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await cg.get_variable(config[CONF_ID])

    for conf in config.get(CONF_ON_BEGIN, []):
        trigger = cg.new_Pvariable(conf[CONF_ON_BEGIN_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_PROGRESS, []):
        trigger = cg.new_Pvariable(conf[CONF_ON_PROGRESS_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(float, "x")], conf)
    for conf in config.get(CONF_ON_END, []):
        trigger = cg.new_Pvariable(conf[CONF_ON_END_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_ERROR, []):
        trigger = cg.new_Pvariable(conf[CONF_ON_ERROR_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(int, "x")], conf)
