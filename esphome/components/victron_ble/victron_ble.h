#pragma once

#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#ifdef USE_ESP32

namespace esphome {
namespace victron_ble {

static constexpr uint8_t VICTRON_MANUFACTURER_RECORD_TYPE = 0x10;

/// Known Victron BLE device record types.
enum class VICTRON_BLE_RECORD_TYPE : uint8_t {
  TEST_RECORD = 0x00,
  SOLAR_CHARGER = 0x01,
  BATTERY_MONITOR = 0x02,
  INVERTER = 0x03,
  DCDC_CONVERTER = 0x04,
  SMART_LITHIUM = 0x05,
  INVERTER_RS = 0x06,
  GX_DEVICE = 0x07,
  AC_CHARGER = 0x08,
  SMART_BATTERY_PROTECT = 0x09,
  LYNX_SMART_BMS = 0x0A,
  MULTI_RS = 0x0B,
  VE_BUS = 0x0C,
  DC_ENERGY_METER = 0x0D,
  ORION_XS = 0x0F,
};

/// Victron device / charger operating states.
enum class VE_REG_DEVICE_STATE : uint8_t {
  OFF = 0x00,
  LOW_POWER = 0x01,
  FAULT = 0x02,
  BULK = 0x03,
  ABSORPTION = 0x04,
  FLOAT = 0x05,
  STORAGE = 0x06,
  EQUALIZE_MANUAL = 0x07,
  EXTERNAL_CONTROL = 0x0B,
  STARTING_UP = 0xF5,
  REPEATED_ABSORPTION = 0xF6,
  AUTO_EQUALIZE = 0xF7,
  BATTERY_SAFE = 0xF8,
  LOAD_DETECT = 0xF9,
  BLOCKED = 0xFA,
  TEST = 0xFB,
  EXTERNAL_CONTROL_2 = 0xFC,
  UNAVAILABLE = 0xFF,
};

/// Victron charger error codes.
enum class VE_REG_CHR_ERROR_CODE : uint8_t {
  NO_ERROR = 0x00,
  ERR_1_BATTERY_TEMPERATURE_TOO_HIGH = 0x01,
  ERR_2_BATTERY_VOLTAGE_TOO_HIGH = 0x02,
  ERR_17_CHARGER_TEMPERATURE_TOO_HIGH = 0x11,
  ERR_18_CHARGER_OVER_CURRENT = 0x12,
  ERR_19_CHARGER_CURRENT_REVERSED = 0x13,
  ERR_20_BULK_TIME_LIMIT_EXCEEDED = 0x14,
  ERR_21_CURRENT_SENSOR_ISSUE = 0x15,
  ERR_26_TERMINALS_OVERHEATED = 0x1A,
  ERR_28_CONVERTER_ISSUE = 0x1C,
  ERR_33_INPUT_VOLTAGE_TOO_HIGH_PV = 0x21,
  ERR_34_INPUT_CURRENT_TOO_HIGH_PV = 0x22,
  ERR_38_INPUT_SHUTDOWN_EXCESSIVE_BATTERY_VOLTAGE = 0x26,
  ERR_39_INPUT_SHUTDOWN_DUE_TO_CURRENT_FLOW = 0x27,
  ERR_65_LOST_COMMUNICATION_WITH_ONE_OF_DEVICES = 0x41,
  ERR_66_SYNCHRONISED_CHARGING_DEVICE_CONFIG_ISSUE = 0x42,
  ERR_67_BMS_CONNECTION_LOST = 0x43,
  ERR_68_NETWORK_MISCONFIGURED = 0x44,
  ERR_116_FACTORY_CALIBRATION_DATA_LOST = 0x74,
  ERR_117_INVALID_FIRMWARE = 0x75,
  ERR_119_SETTINGS_DATA_INVALID = 0x77,
};

/// Packed layout of the Victron manufacturer record (bytes following the Manufacturer ID in
/// the BLE advertisement manufacturer-specific data AD type).
struct VictronManufacturerRecordHeader {
  uint8_t record_type;       ///< Must equal VICTRON_MANUFACTURER_RECORD_TYPE (0x10)
  uint8_t data_length;       ///< Byte count of the remaining payload
  uint16_t product_id;       ///< Product ID, little-endian
  uint8_t device_record_type;  ///< See VICTRON_BLE_RECORD_TYPE
  uint8_t nonce_lsb;         ///< AES-CTR nonce, low byte
  uint8_t nonce_msb;         ///< AES-CTR nonce, high byte
  uint8_t encryption_key_byte;  ///< Byte 0 of the per-device encryption key (used for key validation)
} __attribute__((packed));

/// Base component for Victron BLE devices.
///
/// Handles BLE advertisement scanning, manufacturer data validation, and AES-CTR payload
/// decryption.  Subclasses should override on_victron_ble_record() to consume the decrypted
/// per-record-type data.
class VictronBle : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_address(uint64_t address) { address_ = address; }
  void set_bindkey(const char *bindkey);

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void dump_config() override;

 protected:
  uint64_t address_{0};
  uint8_t bindkey_[16]{};

  /// Decrypt a single AES-CTR block.
  /// @param ciphertext    Input ciphertext (1–16 bytes).
  /// @param cipher_length Number of bytes in ciphertext.
  /// @param nonce_counter 16-byte AES-CTR counter block (nonce padded with zeros).
  /// @param plaintext     Output buffer (at least 16 bytes).
  /// @return true on success, false on failure.
  bool decrypt_payload_(const uint8_t *ciphertext, uint8_t cipher_length, const uint8_t *nonce_counter,
                        uint8_t *plaintext);

  /// Called after successful decryption.  Override in subclasses to consume device-specific
  /// data.  The default implementation does nothing, so the base class can be instantiated on
  /// its own (e.g., for debugging / sniffing) without requiring a subclass.
  virtual void on_victron_ble_record(VICTRON_BLE_RECORD_TYPE record_type, const uint8_t *data, uint8_t length) {}
};

}  // namespace victron_ble
}  // namespace esphome

// Include the Solar Charger subclass so that it is always available when
// the victron_ble component header is included.
#include "victron_ble_solar_charger.h"

#endif  // USE_ESP32
