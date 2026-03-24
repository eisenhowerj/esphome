import esphome.codegen as cg
from esphome.components import esp32_ble_tracker
import esphome.config_validation as cv
from esphome.const import CONF_BINDKEY, CONF_ID, CONF_MAC_ADDRESS

CODEOWNERS = ["@eisenhowerj"]
DEPENDENCIES = ["esp32_ble_tracker"]

victron_ble_ns = cg.esphome_ns.namespace("victron_ble")
VictronBle = victron_ble_ns.class_(
    "VictronBle", cg.Component, esp32_ble_tracker.ESPBTDeviceListener
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VictronBle),
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Required(CONF_BINDKEY): cv.bind_key,
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)
    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    cg.add(var.set_bindkey(config[CONF_BINDKEY]))
