#include "victron_ble_solar_charger.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace victron_ble {

static const char *const TAG = "victron_ble.solar_charger";

#ifdef USE_TEXT_SENSOR

static const char *charge_state_to_string(uint8_t state) {
  switch (static_cast<VE_REG_DEVICE_STATE>(state)) {
    case VE_REG_DEVICE_STATE::OFF:
      return "Off";
    case VE_REG_DEVICE_STATE::LOW_POWER:
      return "Low Power";
    case VE_REG_DEVICE_STATE::FAULT:
      return "Fault";
    case VE_REG_DEVICE_STATE::BULK:
      return "Bulk";
    case VE_REG_DEVICE_STATE::ABSORPTION:
      return "Absorption";
    case VE_REG_DEVICE_STATE::FLOAT:
      return "Float";
    case VE_REG_DEVICE_STATE::STORAGE:
      return "Storage";
    case VE_REG_DEVICE_STATE::EQUALIZE_MANUAL:
      return "Equalize (Manual)";
    case VE_REG_DEVICE_STATE::EXTERNAL_CONTROL:
    case VE_REG_DEVICE_STATE::EXTERNAL_CONTROL_2:
      return "External Control";
    case VE_REG_DEVICE_STATE::STARTING_UP:
      return "Starting Up";
    case VE_REG_DEVICE_STATE::REPEATED_ABSORPTION:
      return "Repeated Absorption";
    case VE_REG_DEVICE_STATE::AUTO_EQUALIZE:
      return "Auto Equalize";
    case VE_REG_DEVICE_STATE::BATTERY_SAFE:
      return "Battery Safe";
    case VE_REG_DEVICE_STATE::LOAD_DETECT:
      return "Load Detect";
    case VE_REG_DEVICE_STATE::BLOCKED:
      return "Blocked";
    case VE_REG_DEVICE_STATE::TEST:
      return "Test";
    case VE_REG_DEVICE_STATE::UNAVAILABLE:
    default:
      return nullptr;
  }
}

static const char *charger_error_to_string(uint8_t error) {
  switch (static_cast<VE_REG_CHR_ERROR_CODE>(error)) {
    case VE_REG_CHR_ERROR_CODE::NO_ERROR:
      return "No Error";
    case VE_REG_CHR_ERROR_CODE::ERR_1_BATTERY_TEMPERATURE_TOO_HIGH:
      return "Err 1: Battery Temperature Too High";
    case VE_REG_CHR_ERROR_CODE::ERR_2_BATTERY_VOLTAGE_TOO_HIGH:
      return "Err 2: Battery Voltage Too High";
    case VE_REG_CHR_ERROR_CODE::ERR_17_CHARGER_TEMPERATURE_TOO_HIGH:
      return "Err 17: Charger Temperature Too High";
    case VE_REG_CHR_ERROR_CODE::ERR_18_CHARGER_OVER_CURRENT:
      return "Err 18: Charger Over Current";
    case VE_REG_CHR_ERROR_CODE::ERR_19_CHARGER_CURRENT_REVERSED:
      return "Err 19: Charger Current Reversed";
    case VE_REG_CHR_ERROR_CODE::ERR_20_BULK_TIME_LIMIT_EXCEEDED:
      return "Err 20: Bulk Time Limit Exceeded";
    case VE_REG_CHR_ERROR_CODE::ERR_21_CURRENT_SENSOR_ISSUE:
      return "Err 21: Current Sensor Issue";
    case VE_REG_CHR_ERROR_CODE::ERR_26_TERMINALS_OVERHEATED:
      return "Err 26: Terminals Overheated";
    case VE_REG_CHR_ERROR_CODE::ERR_28_CONVERTER_ISSUE:
      return "Err 28: Converter Issue";
    case VE_REG_CHR_ERROR_CODE::ERR_33_INPUT_VOLTAGE_TOO_HIGH_PV:
      return "Err 33: Input Voltage Too High (PV)";
    case VE_REG_CHR_ERROR_CODE::ERR_34_INPUT_CURRENT_TOO_HIGH_PV:
      return "Err 34: Input Current Too High (PV)";
    case VE_REG_CHR_ERROR_CODE::ERR_38_INPUT_SHUTDOWN_EXCESSIVE_BATTERY_VOLTAGE:
      return "Err 38: Input Shutdown (Excessive Battery Voltage)";
    case VE_REG_CHR_ERROR_CODE::ERR_39_INPUT_SHUTDOWN_DUE_TO_CURRENT_FLOW:
      return "Err 39: Input Shutdown (Due to Current Flow)";
    case VE_REG_CHR_ERROR_CODE::ERR_65_LOST_COMMUNICATION_WITH_ONE_OF_DEVICES:
      return "Err 65: Lost Communication With One of Devices";
    case VE_REG_CHR_ERROR_CODE::ERR_66_SYNCHRONISED_CHARGING_DEVICE_CONFIG_ISSUE:
      return "Err 66: Synchronised Charging Device Config Issue";
    case VE_REG_CHR_ERROR_CODE::ERR_67_BMS_CONNECTION_LOST:
      return "Err 67: BMS Connection Lost";
    case VE_REG_CHR_ERROR_CODE::ERR_68_NETWORK_MISCONFIGURED:
      return "Err 68: Network Misconfigured";
    case VE_REG_CHR_ERROR_CODE::ERR_116_FACTORY_CALIBRATION_DATA_LOST:
      return "Err 116: Factory Calibration Data Lost";
    case VE_REG_CHR_ERROR_CODE::ERR_117_INVALID_FIRMWARE:
      return "Err 117: Invalid Firmware";
    case VE_REG_CHR_ERROR_CODE::ERR_119_SETTINGS_DATA_INVALID:
      return "Err 119: Settings Data Invalid";
    default:
      return nullptr;
  }
}

#endif  // USE_TEXT_SENSOR

void VictronBleSolarCharger::on_victron_ble_record(VICTRON_BLE_RECORD_TYPE record_type,
                                                   const uint8_t *data, uint8_t length) {
  if (record_type != VICTRON_BLE_RECORD_TYPE::SOLAR_CHARGER) {
    ESP_LOGV(TAG, "on_victron_ble_record(): ignoring record type 0x%02X.",
             static_cast<uint8_t>(record_type));
    return;
  }

  if (length < SOLAR_CHARGER_MIN_LENGTH) {
    ESP_LOGW(TAG, "on_victron_ble_record(): payload too short (%u bytes, need %u).", length,
             SOLAR_CHARGER_MIN_LENGTH);
    return;
  }

  const auto *rec = reinterpret_cast<const VictronSolarChargerRecord *>(data);

  // Charge state (text sensor)
#ifdef USE_TEXT_SENSOR
  if (this->charge_state_ != nullptr) {
    if (rec->charge_state == static_cast<uint8_t>(VE_REG_DEVICE_STATE::UNAVAILABLE)) {
      this->charge_state_->publish_state("");
    } else {
      const char *state_str = charge_state_to_string(rec->charge_state);
      if (state_str != nullptr) {
        this->charge_state_->publish_state(state_str);
      } else {
        this->charge_state_->publish_state("");
        ESP_LOGV(TAG, "on_victron_ble_record(): unknown charge state 0x%02X.", rec->charge_state);
      }
    }
  }

  // Charger error (text sensor)
  if (this->charger_error_ != nullptr) {
    if (rec->charger_error == static_cast<uint8_t>(VE_REG_CHR_ERROR_CODE::NO_ERROR)) {
      this->charger_error_->publish_state("No Error");
    } else if (rec->charger_error == 0xFF) {
      this->charger_error_->publish_state("");
    } else {
      const char *error_str = charger_error_to_string(rec->charger_error);
      if (error_str != nullptr) {
        this->charger_error_->publish_state(error_str);
      } else {
        this->charger_error_->publish_state("");
        ESP_LOGV(TAG, "on_victron_ble_record(): unknown charger error 0x%02X.", rec->charger_error);
      }
    }
  }
#endif  // USE_TEXT_SENSOR

  // Battery voltage (int16, 0.01 V, not-available = 0x7FFF)
#ifdef USE_SENSOR
  auto publish_scaled = [](sensor::Sensor *s, auto raw, decltype(raw) sentinel, float scale) {
    if (s == nullptr)
      return;
    s->publish_state(raw == sentinel ? NAN : static_cast<float>(raw) * scale);
  };

  publish_scaled(this->battery_voltage_, rec->battery_voltage, (int16_t) 0x7FFF, 1.0f / 100.0f);
  publish_scaled(this->battery_charging_current_, rec->battery_charging_current, (int16_t) 0x7FFF,
                 1.0f / 10.0f);
  publish_scaled(this->yield_today_, rec->yield_today, (uint16_t) 0xFFFF, 10.0f);
  publish_scaled(this->solar_power_, rec->solar_power, (uint16_t) 0xFFFF, 1.0f);

  // External device load (uint9 spanning bytes 10–11, 0.1 A, not-available = 0x1FF)
  if (this->external_device_load_ != nullptr) {
    if (length >= SOLAR_CHARGER_LOAD_MIN_LENGTH) {
      // The 9-bit field uses byte[10] as bits 0–7 and bit 0 of byte[11] as bit 8.
      uint16_t raw_load = data[10] | ((data[11] & 0x01) << 8);
      this->external_device_load_->publish_state(raw_load == 0x1FF ? NAN : raw_load / 10.0f);
    } else {
      this->external_device_load_->publish_state(NAN);
    }
  }
#endif  // USE_SENSOR
}

}  // namespace victron_ble
}  // namespace esphome

#endif  // USE_ESP32
