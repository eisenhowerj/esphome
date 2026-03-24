# [victron_ble] Phase 3: Battery Monitor & Inverter Support

## Overview

Extend the `victron_ble` component to support additional Victron device types: Battery Monitor (record type 0x02) and Inverter (record type 0x03). These share the same BLE infrastructure built in Phase 1.

## Prerequisites

- Phase 1: Core BLE Infrastructure (completed)
- Phase 2: Solar Charger Sensors (completed — establishes the sensor/text_sensor pattern)

## Battery Monitor Record Format (Type 0x02)

| Bits    | Field                    | Type     | Unit / Scale                  | "Not Available" Value |
|---------|--------------------------|----------|-------------------------------|-----------------------|
| 0-15    | Time To Go               | uint16   | 1 minute increments           | 0xFFFF                |
| 16-31   | Battery Voltage          | int16    | 0.01 V increments             | 0x7FFF                |
| 32-41   | Alarm Reason             | uint10   | Bitfield                      | —                     |
| 42-63   | Aux Voltage / Temp / Mid | varies   | Depends on aux input mode     | varies                |
| 64-85   | Battery Current          | int22    | 0.001 A increments            | 0x3FFFFF              |
| 86-95   | Consumed Ah              | uint10   | varies                        | 0x3FF                 |
| 96-106  | State of Charge          | uint10   | 0.1% increments               | 0x3FF                 |

### Sensors to Expose
- `battery_voltage` (V)
- `battery_current` (A)
- `state_of_charge` (%)
- `consumed_ah` (Ah)
- `time_to_go` (minutes)
- `temperature` (°C, when aux input is temperature)
- `midpoint_voltage` (V, when aux input is midpoint)
- `starter_voltage` (V, when aux input is starter battery)

### Text Sensors
- `alarm_reason` — Human-readable alarm description

### Supported Devices
- BMV-700, BMV-702, BMV-700H
- BMV-712 Smart, BMV-710H Smart
- SmartShunt 500A/50mV, 1000A/50mV, 2000A/50mV

---

## Inverter Record Format (Type 0x03)

| Bits    | Field                    | Type     | Unit / Scale                  | "Not Available" Value |
|---------|--------------------------|----------|-------------------------------|-----------------------|
| 0-7     | Device State             | uint8    | Enum (VE_REG_DEVICE_STATE)    | 0xFF                  |
| 8-23    | Alarm Reason             | uint16   | Bitfield                      | —                     |
| 24-39   | Battery Voltage          | int16    | 0.01 V increments             | 0x7FFF                |
| 40-55   | AC Apparent Power        | uint16   | 1 VA increments               | 0xFFFF                |
| 56-70   | AC Voltage               | uint15   | 0.01 V increments             | 0x7FFF                |
| 71-81   | AC Current               | uint11   | 0.1 A increments              | 0x7FF                 |

### Sensors to Expose
- `battery_voltage` (V)
- `ac_apparent_power` (VA)
- `ac_voltage` (V)
- `ac_current` (A)

### Text Sensors
- `device_state` — Human-readable device state
- `alarm_reason` — Human-readable alarm description

### Supported Devices
- Phoenix Inverter 12V/24V/48V in various power ratings (250VA-3000VA)
- Both 230V and 120V variants

---

## Tasks

### Battery Monitor
- [ ] Create `VictronBleBatteryMonitor` class
- [ ] Implement record type 0x02 parsing with bit-field extraction
- [ ] Handle aux input modes (temperature, midpoint voltage, starter voltage)
- [ ] Add sensor and text_sensor platform entries
- [ ] Map alarm reason bitfield to human-readable strings

### Inverter
- [ ] Create `VictronBleInverter` class
- [ ] Implement record type 0x03 parsing
- [ ] Add sensor and text_sensor platform entries
- [ ] Map alarm reason bitfield to human-readable strings

### Python Configuration
- [ ] Add device type selection or auto-detection based on record type
- [ ] Extend sensor.py and text_sensor.py for new device types
- [ ] Add validation to prevent configuring sensors not applicable to the device type

## Acceptance Criteria

- [ ] Battery Monitor sensors correctly report all values matching VictronConnect
- [ ] Inverter sensors correctly report all values matching VictronConnect
- [ ] Aux input mode is handled correctly for Battery Monitor
- [ ] Alarm reasons are properly decoded
- [ ] Configuration validation prevents invalid sensor combinations
