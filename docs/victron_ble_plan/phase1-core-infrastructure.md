# [victron_ble] Phase 1: Core BLE Infrastructure & AES-CTR Decryption

## Overview

Add the core `victron_ble` component that handles BLE advertisement scanning, manufacturer data parsing, and AES-CTR decryption for Victron Energy devices.

## Background

Victron Energy devices (SmartSolar, SmartShunt, Phoenix Inverters, etc.) broadcast encrypted telemetry via BLE manufacturer data. The manufacturer ID is `0x02E1`. Data is encrypted using AES-CTR with a per-device encryption key obtained from the VictronConnect app.

### BLE Advertisement Structure

```
Manufacturer Data (type 0xFF):
├── Manufacturer ID: 0x02E1 (Victron Energy BV)
├── Manufacturer Record Type: 0x10 (Product Advertisement)
├── Record Length
├── Product ID (16-bit, little-endian)
├── Record Type (8-bit): identifies device type
│   (0x01=Solar Charger, 0x02=Battery Monitor, 0x03=Inverter, etc.)
├── Data Counter LSB (8-bit) ─┐
├── Data Counter MSB (8-bit) ─┘ 16-bit nonce for AES-CTR
├── Encryption Key Byte 0 (8-bit): for key validation
└── Encrypted Payload (up to 16 bytes)
```

## Tasks

### Python Configuration (`__init__.py`)

- [ ] Define `victron_ble` namespace and `VictronBle` class extending `ESPBTDeviceListener` and `Component`
- [ ] Add config schema with:
  - `mac_address` (required): Device MAC address
  - `bindkey` (required): 32-character hex string (16-byte AES-128 key)
- [ ] Extend `ESP_BLE_DEVICE_SCHEMA` and `COMPONENT_SCHEMA`
- [ ] Register component as BLE device listener
- [ ] Generate code to pass MAC address and bindkey to C++ class

### C++ Core (`victron_ble.h` / `victron_ble.cpp`)

- [ ] Define `VICTRON_MANUFACTURER_ID = 0x02E1`
- [ ] Define packed structs for manufacturer data header and record base
- [ ] Define `VICTRON_BLE_RECORD_TYPE` enum covering all known device types:
  - 0x00: Test Record
  - 0x01: Solar Charger
  - 0x02: Battery Monitor
  - 0x03: Inverter
  - 0x04: DC/DC Converter
  - 0x05: Smart Lithium
  - 0x06: Inverter RS
  - 0x07: GX Device
  - 0x08: AC Charger
  - 0x09: Smart Battery Protect
  - 0x0A: Lynx Smart BMS
  - 0x0B: Multi RS
  - 0x0C: VE.Bus
  - 0x0D: DC Energy Meter
  - 0x0F: Orion XS
- [ ] Define `VE_REG_DEVICE_STATE` enum for charge/device states
- [ ] Define `VE_REG_CHR_ERROR_CODE` enum for charger error codes
- [ ] Implement `parse_device()` override:
  - Filter by MAC address
  - Validate manufacturer ID (`0x02E1`)
  - Validate manufacturer record type (`0x10`)
  - Extract record type, nonce (data counter), and encryption key byte 0
  - Validate encryption key byte 0 matches configured bindkey
- [ ] Implement AES-CTR decryption using mbedtls (already available in ESP-IDF):
  - Build 16-byte counter block from 16-bit nonce
  - Single-block AES-CTR decryption (payload ≤ 16 bytes)
- [ ] Define virtual/callback method for subclasses to handle decrypted data per record type
- [ ] Add logging (ESP_LOGD/ESP_LOGV) for debugging advertisement parsing

## Reference Implementation

- [victron-ble Python library](https://github.com/keshavdv/victron-ble) — AES-CTR decryption and record parsing reference
- [esphome-victron_ble external component](https://github.com/Fabian-Schmidt/esphome-victron_ble) — Existing ESPHome external component reference
- [Victron Extra Manufacturer Data PDF](https://communityarchive.victronenergy.com/storage/attachments/extra-manufacturer-data-2022-12-14.pdf) — Official protocol specification
- [Victron BLE Advertising Arduino Example](https://github.com/hoberman/Victron_BLE_Advertising_example) — C++ parsing reference

## Component File Structure

```
esphome/components/victron_ble/
├── __init__.py          # Python config validation & code generation
├── victron_ble.h        # C++ header: structs, enums, class declaration
└── victron_ble.cpp      # C++ implementation: BLE parsing, AES-CTR decryption
```

## Dependencies

- `esp32_ble_tracker` — BLE advertisement scanning and device listener registration
- `mbedtls` — AES-CTR encryption/decryption (bundled with ESP-IDF, no extra dependency needed)

## Platform Support

- ESP32 only (requires BLE stack)

## Acceptance Criteria

- [ ] Component compiles successfully for ESP32 platform
- [ ] Filters BLE advertisements by configured MAC address and Victron manufacturer ID
- [ ] Validates encryption key byte 0 against configured bindkey
- [ ] Successfully decrypts AES-CTR encrypted payload
- [ ] Provides clean interface for record-type-specific subclasses to implement parsing
- [ ] Includes appropriate debug logging
