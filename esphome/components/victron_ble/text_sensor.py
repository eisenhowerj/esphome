import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv

from . import CONF_VICTRON_BLE_ID, VictronBleSolarCharger

CODEOWNERS = ["@eisenhowerj"]
DEPENDENCIES = ["victron_ble"]

CONF_CHARGE_STATE = "charge_state"
CONF_CHARGER_ERROR = "charger_error"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_VICTRON_BLE_ID): cv.use_id(VictronBleSolarCharger),
        cv.Optional(CONF_CHARGE_STATE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_CHARGER_ERROR): text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    var = await cg.get_variable(config[CONF_VICTRON_BLE_ID])

    if charge_state_config := config.get(CONF_CHARGE_STATE):
        sens = await text_sensor.new_text_sensor(charge_state_config)
        cg.add(var.set_charge_state(sens))
    if charger_error_config := config.get(CONF_CHARGER_ERROR):
        sens = await text_sensor.new_text_sensor(charger_error_config)
        cg.add(var.set_charger_error(sens))
