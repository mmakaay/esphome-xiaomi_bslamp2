import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.effects import register_monochromatic_effect
from esphome.components.light.types import light_ns, LightEffect
from esphome.const import CONF_NAME

CODEOWNERS = ["@mmakaay"]

CandleLightEffect = light_ns.class_("CandleLightEffect", LightEffect)

CONFIG_SCHEMA = cv.Schema({})

@register_monochromatic_effect(
    "candle",
    CandleLightEffect,
    "Candle",
    {
    },
)
async def candle_effect_to_code(config, effect_id):
    effect = cg.new_Pvariable(effect_id, config[CONF_NAME])
    return effect

