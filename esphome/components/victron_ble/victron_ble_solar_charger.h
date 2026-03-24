#pragma once

#include "victron_ble.h"

#ifdef USE_ESP32

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

namespace esphome {
namespace victron_ble {

/// Packed layout of the Solar Charger record (type 0x01) after AES-CTR decryption.
struct VictronSolarChargerRecord {
  uint8_t charge_state;              ///< VE_REG_DEVICE_STATE; 0xFF = not available
  uint8_t charger_error;             ///< VE_REG_CHR_ERROR_CODE; 0xFF = not available
  int16_t battery_voltage;           ///< 0.01 V increments; 0x7FFF = not available
  int16_t battery_charging_current;  ///< 0.1 A increments; 0x7FFF = not available
  uint16_t yield_today;              ///< 10 Wh increments; 0xFFFF = not available
  uint16_t solar_power;              ///< 1 W increments; 0xFFFF = not available
  // External device load is 9 bits spanning bytes 10–11
} __attribute__((packed));

/// Minimum payload bytes required to parse through solar_power (bytes 0–9).
static constexpr uint8_t SOLAR_CHARGER_MIN_LENGTH = 10;
/// Minimum payload bytes required to parse through external_device_load (bytes 0–11).
static constexpr uint8_t SOLAR_CHARGER_LOAD_MIN_LENGTH = 11;

/// ESPHome component for Victron SmartSolar / BlueSolar MPPT charge controllers.
///
/// Subscribes to Solar Charger BLE advertisement records (type 0x01), decrypts
/// the payload via the base-class AES-CTR implementation, and publishes sensor
/// values.
class VictronBleSolarCharger : public VictronBle {
 public:
#ifdef USE_SENSOR
  void set_battery_voltage(sensor::Sensor *s) { battery_voltage_ = s; }
  void set_battery_charging_current(sensor::Sensor *s) { battery_charging_current_ = s; }
  void set_yield_today(sensor::Sensor *s) { yield_today_ = s; }
  void set_solar_power(sensor::Sensor *s) { solar_power_ = s; }
  void set_external_device_load(sensor::Sensor *s) { external_device_load_ = s; }
#endif
#ifdef USE_TEXT_SENSOR
  void set_charge_state(text_sensor::TextSensor *s) { charge_state_ = s; }
  void set_charger_error(text_sensor::TextSensor *s) { charger_error_ = s; }
#endif

 protected:
  void on_victron_ble_record(VICTRON_BLE_RECORD_TYPE record_type, const uint8_t *data,
                             uint8_t length) override;

#ifdef USE_SENSOR
  sensor::Sensor *battery_voltage_{nullptr};
  sensor::Sensor *battery_charging_current_{nullptr};
  sensor::Sensor *yield_today_{nullptr};
  sensor::Sensor *solar_power_{nullptr};
  sensor::Sensor *external_device_load_{nullptr};
#endif
#ifdef USE_TEXT_SENSOR
  text_sensor::TextSensor *charge_state_{nullptr};
  text_sensor::TextSensor *charger_error_{nullptr};
#endif
};

}  // namespace victron_ble
}  // namespace esphome

#endif  // USE_ESP32
