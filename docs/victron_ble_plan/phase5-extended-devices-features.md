# [victron_ble] Phase 5: Extended Device Support & Features

## Overview

Extend the `victron_ble` component with support for additional Victron device types and advanced features.

## Prerequisites

- Phases 1-4 completed

## Additional Device Types

### DC/DC Converter (Record Type 0x04)
- [ ] Parse DC/DC converter data fields
- [ ] Sensors: input voltage, output voltage, off reason
- [ ] Text sensors: device state, charger error

### Smart Lithium (Record Type 0x05)
- [ ] Parse Smart Lithium BMS data
- [ ] Sensors: battery voltage, battery temperature, cell voltages
- [ ] Text sensors: balancer status

### AC Charger (Record Type 0x08)
- [ ] Parse AC charger data
- [ ] Sensors: battery voltage, battery current, AC current
- [ ] Text sensors: device state, charger error

### Smart Battery Protect (Record Type 0x09)
- [ ] Parse Smart Battery Protect data
- [ ] Sensors: battery voltage, output voltage
- [ ] Text sensors: device state, error code, off reason

### DC Energy Meter (Record Type 0x0D)
- [ ] Parse DC Energy Meter data
- [ ] Sensors: voltage, current, power, energy
- [ ] Text sensors: meter type

### Orion XS (Record Type 0x0F)
- [ ] Parse Orion XS DC/DC converter data
- [ ] Sensors: input/output voltage, current
- [ ] Text sensors: device state, error code, off reason

---

## Advanced Features

### Binary Sensors
- [ ] Add `binary_sensor` platform for boolean status flags:
  - `has_error` — True when charger error is non-zero
  - `is_charging` — True when charge state is Bulk/Absorption/Float
  - `alarm_active` — True when any alarm is active (Battery Monitor/Inverter)

### Device Auto-Detection
- [ ] Automatically determine device type from the record type byte in BLE advertisement
- [ ] Log detected product ID and device name for easy identification
- [ ] Support scanning mode to discover Victron devices (similar to `esp32_ble_tracker` scanner)

### Automation Triggers
- [ ] Add `on_message` trigger for raw data access in automations
- [ ] Add `on_state_change` trigger for charge state transitions
- [ ] Add `on_error` trigger for charger error events

### Diagnostic Sensors
- [ ] RSSI (signal strength) sensor
- [ ] Last update timestamp sensor
- [ ] Data counter (to detect missed updates)
- [ ] Product ID / model name text sensor

### Multi-Device Support
- [ ] Support monitoring multiple Victron devices simultaneously
- [ ] Each device instance has its own MAC address and bindkey
- [ ] Verify performance with 3+ devices being monitored

---

## YAML Configuration Example (Multi-Device)

```yaml
esp32_ble_tracker:

sensor:
  - platform: victron_ble
    mac_address: "AA:BB:CC:DD:EE:01"
    bindkey: "key1..."
    type: solar_charger
    battery_voltage:
      name: "MPPT Battery Voltage"
    solar_power:
      name: "MPPT Solar Power"

  - platform: victron_ble
    mac_address: "AA:BB:CC:DD:EE:02"
    bindkey: "key2..."
    type: battery_monitor
    battery_voltage:
      name: "Shunt Battery Voltage"
    battery_current:
      name: "Shunt Battery Current"
    state_of_charge:
      name: "Battery SoC"

binary_sensor:
  - platform: victron_ble
    mac_address: "AA:BB:CC:DD:EE:01"
    has_error:
      name: "MPPT Has Error"
    is_charging:
      name: "MPPT Is Charging"
```

## Acceptance Criteria

- [ ] All additional device types parse correctly
- [ ] Binary sensors work reliably
- [ ] Multi-device monitoring performs well
- [ ] Automation triggers fire correctly
- [ ] Documentation updated for all new features
