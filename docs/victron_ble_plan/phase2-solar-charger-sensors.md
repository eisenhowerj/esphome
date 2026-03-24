# [victron_ble] Phase 2: Solar Charger Sensor Support (Record Type 0x01)

## Overview

Add sensor support for Victron SmartSolar / BlueSolar MPPT charge controllers, parsing the Solar Charger BLE advertisement record (type 0x01) and exposing key telemetry as ESPHome sensors.

## Prerequisites

- Phase 1: Core BLE Infrastructure & AES-CTR Decryption (must be completed first)

## Solar Charger Record Format (Type 0x01)

After AES-CTR decryption, the 16-byte payload contains:

| Bits    | Field                      | Type     | Unit / Scale                  | "Not Available" Value |
|---------|----------------------------|----------|-------------------------------|-----------------------|
| 0-7     | Charge State               | uint8    | Enum (VE_REG_DEVICE_STATE)    | 0xFF                  |
| 8-15    | Charger Error              | uint8    | Enum (VE_REG_CHR_ERROR_CODE)  | 0xFF                  |
| 16-31   | Battery Voltage            | int16    | 0.01 V increments             | 0x7FFF                |
| 32-47   | Battery Charging Current   | int16    | 0.1 A increments              | 0x7FFF                |
| 48-63   | Yield Today                | uint16   | 10 Wh increments              | 0xFFFF                |
| 64-79   | Solar Power                | uint16   | 1 W increments                | 0xFFFF                |
| 80-88   | External Device Load       | uint9    | 0.1 A increments              | 0x1FF                 |

## Tasks

### Python Sensor Configuration (`sensor.py`)

- [ ] Create sensor platform file for `victron_ble`
- [ ] Define `VictronBleSolarCharger` class extending base `VictronBle`
- [ ] Add config schema with optional sensor entries:
  - `battery_voltage` — unit: V, accuracy: 2 decimals, device_class: voltage
  - `battery_charging_current` — unit: A, accuracy: 1 decimal, device_class: current
  - `yield_today` — unit: Wh, accuracy: 0 decimals, device_class: energy
  - `solar_power` — unit: W, accuracy: 0 decimals, device_class: power
  - `external_device_load` — unit: A, accuracy: 1 decimal, device_class: current
- [ ] Generate code to create sensor objects and pass to C++ class via setters

### Python Text Sensor Configuration (`text_sensor.py`)

- [ ] Define text sensor platform for status fields:
  - `charge_state` — Human-readable charge state (Off, Bulk, Absorption, Float, etc.)
  - `charger_error` — Human-readable error description

### C++ Solar Charger Implementation

- [ ] Define `VictronBleSolarCharger` class:
  - Inherit from base `VictronBle` component
  - Override decrypted data handler for record type 0x01
- [ ] Implement Solar Charger record parsing:
  - Parse bit-packed fields per specification
  - Handle "not available" sentinel values (publish NAN or skip)
  - Apply scaling factors (÷100 for voltage, ÷10 for current, ×10 for yield)
- [ ] Add sensor pointer members with setters:
  - `set_battery_voltage(sensor::Sensor *)`
  - `set_battery_charging_current(sensor::Sensor *)`
  - `set_yield_today(sensor::Sensor *)`
  - `set_solar_power(sensor::Sensor *)`
  - `set_external_device_load(sensor::Sensor *)`
- [ ] Add text sensor pointer members with setters:
  - `set_charge_state(text_sensor::TextSensor *)`
  - `set_charger_error(text_sensor::TextSensor *)`
- [ ] Publish sensor values via `sensor->publish_state(value)`
- [ ] Map enum values to human-readable strings for text sensors

### YAML Configuration Example

```yaml
esp32_ble_tracker:

sensor:
  - platform: victron_ble
    mac_address: "AA:BB:CC:DD:EE:FF"
    bindkey: "0df4d0395b7d1a876c0c33ecb9e70dcd"
    battery_voltage:
      name: "SmartSolar Battery Voltage"
    battery_charging_current:
      name: "SmartSolar Charging Current"
    yield_today:
      name: "SmartSolar Yield Today"
    solar_power:
      name: "SmartSolar Solar Power"
    external_device_load:
      name: "SmartSolar Load Current"

text_sensor:
  - platform: victron_ble
    charge_state:
      name: "SmartSolar Charge State"
    charger_error:
      name: "SmartSolar Charger Error"
```

## Supported Devices

All Victron SmartSolar and BlueSolar MPPT charge controllers that support BLE Instant Readout, including:
- SmartSolar MPPT 75/10, 75/15, 100/15, 100/20, 100/30, 100/50
- SmartSolar MPPT 150/35, 150/45, 150/60, 150/70, 150/85, 150/100
- SmartSolar MPPT 250/45, 250/60, 250/70, 250/85, 250/100
- SmartSolar MPPT VE.Can variants
- BlueSolar MPPT variants (with SmartSolar dongle)

## Charge State Values

| Value | State              |
|-------|--------------------|
| 0x00  | Off                |
| 0x01  | Low Power          |
| 0x02  | Fault              |
| 0x03  | Bulk               |
| 0x04  | Absorption         |
| 0x05  | Float              |
| 0x06  | Storage            |
| 0x07  | Equalize (Manual)  |
| 0xF5  | Starting Up        |
| 0xF6  | Repeated Absorption|
| 0xF7  | Auto Equalize      |
| 0xF8  | Battery Safe       |
| 0xF9  | Load Detect        |
| 0xFA  | Blocked            |
| 0xFB  | Test               |
| 0xFC  | External Control   |
| 0xFF  | Not Available      |

## Acceptance Criteria

- [ ] Solar Charger sensors correctly report battery voltage, charging current, yield, solar power, and load
- [ ] Text sensors correctly report human-readable charge state and error descriptions
- [ ] "Not available" sentinel values are handled gracefully (NAN or empty string)
- [ ] Sensor values match those shown in VictronConnect app
- [ ] YAML configuration example compiles and works
