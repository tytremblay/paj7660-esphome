import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, text_sensor

from .. import pag7660_ns

DEPENDENCIES = ["i2c"]
MULTI_CONF = True

CONF_GESTURE_MODE = "gesture_mode"

GESTURE_MODES = {
    "thumb": 2,
    "cursor": 4,
    "combined": 5,
}

PAG7660TextSensor = pag7660_ns.class_(
    "PAG7660TextSensor",
    text_sensor.TextSensor,
    cg.PollingComponent,
    i2c.I2CDevice,
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(
        PAG7660TextSensor,
        icon="mdi:gesture-swipe",
    )
    .extend(cv.polling_component_schema("100ms"))
    .extend(i2c.i2c_device_schema(0x68))
    .extend(
        {
            cv.Optional(CONF_GESTURE_MODE, default="combined"): cv.enum(
                GESTURE_MODES, lower=True
            ),
        }
    )
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_gesture_mode(config[CONF_GESTURE_MODE]))
