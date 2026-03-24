# Victron Energy BLE Component — Implementation Plan

## Summary

This plan adds a native `victron_ble` ESPHome component for monitoring Victron Energy devices (SmartSolar MPPT charge controllers, SmartShunts, Phoenix Inverters, etc.) via Bluetooth Low Energy (BLE) advertisement data. The component parses encrypted manufacturer data broadcast by Victron devices and exposes telemetry as ESPHome sensors.

## Architecture

The component follows the established ESPHome BLE sensor pattern (similar to `xiaomi_ble`, `mopeka_ble`, `thermopro_ble`):

```
┌─────────────────────────────────────────────────┐
│ esp32_ble_tracker                                │
│   Scans BLE advertisements                       │
│   Calls parse_device() on registered listeners   │
└──────────────────┬──────────────────────────────┘
                   │
         ┌─────────▼──────────┐
         │ VictronBle (base)  │
         │  - MAC filtering    │
         │  - Mfr ID check     │
         │  - AES-CTR decrypt  │
         └─────────┬──────────┘
                   │
    ┌──────────────┼──────────────┐
    │              │              │
    ▼              ▼              ▼
Solar Charger  Battery Mon.  Inverter ...
(type 0x01)   (type 0x02)  (type 0x03)
```

### Component File Structure

```
esphome/components/victron_ble/
├── __init__.py              # Core Python config, base component
├── sensor.py                # Sensor platform (voltage, current, power, etc.)
├── text_sensor.py           # Text sensor platform (charge state, errors)
├── binary_sensor.py         # Binary sensor platform (Phase 5)
├── victron_ble.h            # C++ header: enums, structs, base class
├── victron_ble.cpp           # C++ implementation: BLE parsing, AES-CTR
└── victron_ble_solar_charger.h  # Solar charger specific parsing (optional)
```

### Key Technical Details

| Aspect                  | Detail                                                    |
|-------------------------|-----------------------------------------------------------|
| **Manufacturer ID**     | `0x02E1` (Victron Energy BV)                              |
| **Encryption**          | AES-128-CTR, per-device key from VictronConnect app       |
| **Max Payload Size**    | 16 bytes (single AES block)                               |
| **Nonce**               | 16-bit data counter from advertisement header             |
| **Key Validation**      | Byte 0 of key is broadcast for quick validation           |
| **Framework**           | ESP32 only (requires BLE)                                 |
| **Dependencies**        | `esp32_ble_tracker`, mbedtls (bundled with ESP-IDF)       |

## Phases

Each phase corresponds to a GitHub Issue (see individual files for full details):

### Phase 1: Core BLE Infrastructure & AES-CTR Decryption
**[phase1-core-infrastructure.md](phase1-core-infrastructure.md)**

Build the foundation: BLE advertisement filtering, manufacturer data parsing, AES-CTR decryption, and the extensible base class for device-specific parsers.

### Phase 2: Solar Charger Sensor Support (Record Type 0x01)
**[phase2-solar-charger-sensors.md](phase2-solar-charger-sensors.md)**

Parse SmartSolar/BlueSolar MPPT data and expose sensors: battery voltage, charging current, yield today, solar power, external load current, charge state, and charger error.

### Phase 3: Battery Monitor & Inverter Support
**[phase3-battery-monitor-inverter.md](phase3-battery-monitor-inverter.md)**

Add Battery Monitor (BMV/SmartShunt) and Phoenix Inverter support with their respective sensor sets.

### Phase 4: Testing & Documentation
**[phase4-testing-documentation.md](phase4-testing-documentation.md)**

Comprehensive test coverage (YAML compilation tests, unit tests), component documentation, and CI validation.

### Phase 5: Extended Device Support & Features
**[phase5-extended-devices-features.md](phase5-extended-devices-features.md)**

Additional device types (DC/DC converter, Smart Lithium, AC Charger, etc.), binary sensors, automation triggers, and multi-device support.

## References

- [Victron Extra Manufacturer Data Specification (PDF)](https://communityarchive.victronenergy.com/storage/attachments/extra-manufacturer-data-2022-12-14.pdf)
- [victron-ble Python Library](https://github.com/keshavdv/victron-ble) — Reference implementation for decryption and parsing
- [esphome-victron_ble External Component](https://github.com/Fabian-Schmidt/esphome-victron_ble) — Community ESPHome integration
- [Victron BLE Arduino Example](https://github.com/hoberman/Victron_BLE_Advertising_example) — C++ parsing reference
- [Victron Community Discussion](https://community.victronenergy.com/questions/187303/victron-bluetooth-advertising-protocol.html)

## Creating GitHub Issues

To create the GitHub issues from these plan documents, run the following for each phase:

```bash
gh issue create --repo <owner>/esphome \
  --title "<title from markdown file>" \
  --body-file docs/victron_ble_plan/<phase-file>.md
```
