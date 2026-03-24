# [victron_ble] Phase 4: Testing & Documentation

## Overview

Add comprehensive tests and documentation for the `victron_ble` component to ensure reliability and ease of adoption.

## Prerequisites

- Phase 1-3 completed (core infrastructure, solar charger, battery monitor, inverter)

## Tasks

### Unit Tests

- [ ] Add YAML compilation tests in `tests/` directory:
  - Test configuration with all sensor types for Solar Charger
  - Test configuration with all sensor types for Battery Monitor
  - Test configuration with all sensor types for Inverter
  - Test minimal configuration (MAC + bindkey only)
  - Test configuration validation (invalid bindkey length, missing required fields)
- [ ] Follow existing test patterns (see `tests/components/` for examples)

### Integration Testing

- [ ] Create test YAML files that can be compiled for ESP32:
  - `tests/components/victron_ble/test.esp32-idf.yaml`
  - `tests/components/victron_ble/test.esp32-ard.yaml` (if Arduino framework is supported)
- [ ] Verify component compiles with both ESP-IDF and Arduino frameworks (if applicable)

### C++ Unit Tests (if applicable)

- [ ] Test AES-CTR decryption with known test vectors
- [ ] Test Solar Charger record parsing with known byte sequences
- [ ] Test Battery Monitor record parsing with known byte sequences
- [ ] Test handling of "not available" sentinel values
- [ ] Test bit-field extraction edge cases

### Documentation

- [ ] Create component documentation following ESPHome docs format:
  - Component overview and purpose
  - Hardware requirements (ESP32 with BLE)
  - How to obtain the encryption key from VictronConnect app
  - Configuration variables reference
  - Full YAML configuration examples for each device type
  - Troubleshooting section (common issues, debugging tips)
- [ ] Add supported device list with product IDs
- [ ] Document sensor units, accuracy, and update frequency
- [ ] Add wiring/setup notes (no physical wiring needed — BLE only)

### Code Quality

- [ ] Run `ruff` linting on all Python files
- [ ] Run `clang-format` on all C++ files
- [ ] Ensure all code follows ESPHome coding conventions
- [ ] Add appropriate `CODEOWNERS` entry for the component
- [ ] Verify no compiler warnings on ESP32

## Acceptance Criteria

- [ ] All test YAML files compile successfully
- [ ] Linting passes with zero errors
- [ ] Documentation is clear and complete
- [ ] CODEOWNERS file updated
- [ ] CI pipeline passes
