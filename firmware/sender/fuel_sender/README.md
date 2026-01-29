# ESP32C6 Fuel Sender Firmware

This folder contains the firmware for the **fuel tank level monitoring** sender unit. Designed for a 1972 VW Superbeetle with a variable resistance fuel sender (73Ω empty → 10Ω full).

## Overview

The fuel sender unit reads the fuel tank's variable resistance sensor and transmits the calculated fuel percentage (0-100%) wirelessly via ESP-NOW to the CYD display at 1 Hz.

## Files

- **fuel_sender.ino** - Main Arduino sketch (ADC reading, ESP-NOW transmission, packet handling)
- **fuel_config.h** - Pin definitions, timing constants, and calibration parameters
- **fuel_data_packet.h** - ESP-NOW data packet structure (Protocol v1) with checksum helpers
- **fuel_calibration.cpp** - Interactive serial calibration menu and Preferences storage

## Hardware

- Seeed Studio XIAO ESP32C6 (Cortex-M4 240MHz, 2.4GHz WiFi)
- 1972 VW Superbeetle fuel sender (variable resistor: 73Ω empty → 10Ω full)
- 100Ω 1/4W resistor (voltage divider series element)
- 100nF ceramic capacitor (noise filtering)
- SSD1306 OLED Display (128x64, I2C) - Optional for local display

See [../../hardware/sender/fuel_sender/wiring.md](../../hardware/sender/fuel_sender/wiring.md) for complete wiring diagrams and resistor selection.

## Features

- **Resistance-to-Percentage Conversion**
  - 1972 VW spec: 73Ω (empty) → 10Ω (full)
  - Linear mapping with configurable offsets
  - Supports two-point field calibration (±2% accuracy)

- **Voltage Divider ADC Circuit**
  - 100Ω series resistor + 100nF noise filter
  - 12-bit ADC input on GPIO3 (0-3.3V range)
  - Optimized voltage range: 0.3V-1.9V (10Ω-73Ω)

- **Exponential Smoothing**
  - ALPHA = 0.2 (same as oil sender)
  - Clean, jitter-free fuel readings
  - Responsive to rapid tank level changes

- **Fault Detection**
  - Open circuit detection (disconnected sender)
  - Short circuit detection (resistance too low)
  - Low fuel warning (configurable threshold, default 15%)
  - Transmits fault status in packet

- **ESP-NOW Communication**
  - Protocol v1 (separate from oil sender's v3)
  - 1 Hz transmission rate
  - Checksum validation
  - Automatic 3-attempt retry on failure
  - Independent from oil sender communication

- **Serial Calibration Menu**
  - Interactive menu at 115200 baud
  - Two-point calibration (RECOMMENDED)
  - Single-point calibration option
  - Manual offset adjustment
  - Low fuel threshold configuration
  - Persistent storage in ESP32 flash (Preferences)

- **Local Display Support**
  - Optional SSD1306 OLED display
  - Shows fuel %, resistance, fault codes
  - 10 Hz update rate

## Required Libraries

Install these libraries via Arduino Library Manager:

```
Preferences (built-in ESP32)
WiFi (built-in ESP32)
Wire (built-in)
esp_now.h (built-in ESP32)
```

Note: OLED display support is optional. If enabled in fuel_config.h, also install:
- Adafruit SSD1306
- Adafruit GFX Library

## Configuration

### Pin Assignments (fuel_config.h)

**ADC Input:**
- GPIO3 (XIAO D8) - Fuel sender voltage divider input

**I2C (for optional OLED):**
- GPIO1 (XIAO D1) - SDA
- GPIO2 (XIAO D2) - SCL

**Serial (for calibration menu):**
- GPIO20 (XIAO D0) - RX
- GPIO21 (XIAO D0) - TX

### Fuel Sender Specifications (fuel_config.h)

**VW Superbeetle (1972 spec):**
```cpp
#define FUEL_OHMS_EMPTY 73         // Empty tank resistance
#define FUEL_OHMS_FULL 10          // Full tank resistance
#define FUEL_RESISTANCE_RANGE 63   // (73 - 10)
```

**Voltage Divider:**
```cpp
#define VOLTAGE_DIVIDER_SERIES 100.0    // Series resistor (ohms)
#define VOLTAGE_DIVIDER_VCC 3.3         // Supply voltage
```

### Receiver MAC Address (CRITICAL)

Update the CYD display MAC address in fuel_config.h:

```cpp
#define CYD_MAC_ADDR {0x08, 0xD1, 0xF9, 0x2A, 0x0B, 0xBC}
```

**To find your CYD's MAC:**
1. Upload `Get_MAC_Address.ino` from `../display/` to CYD
2. Open Serial Monitor (115200 baud)
3. Note the displayed MAC address
4. Update CYD_MAC_ADDR in fuel_config.h

See [../../docs/setup/espnow-setup.md](../../docs/setup/espnow-setup.md) for details.

### Timing Configuration (fuel_config.h)

```cpp
#define SAMPLE_INTERVAL_MS 500           // ADC read rate (2 Hz)
#define TRANSMIT_INTERVAL_MS 1000        // Packet send rate (1 Hz)
#define ESPNOW_RETRY_ATTEMPTS 3          // Retries on failed send
#define ESPNOW_RETRY_DELAY_MS 50         // Delay between retries
```

### Fault Detection Thresholds (fuel_config.h)

```cpp
#define FUEL_FAULT_OPEN_CIRCUIT_OHMS 100.0     // > 100Ω triggers fault
#define FUEL_FAULT_SHORT_CIRCUIT_OHMS 5.0      // < 5Ω triggers fault
#define LOW_FUEL_THRESHOLD_PERCENT 15          // Alert below 15%
```

## Upload Instructions

### Using Arduino IDE

1. **Install ESP32 board support:**
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "esp32"

2. **Select board:**
   - Tools → Board → esp32 → "XIAO_ESP32C6"

3. **Configure settings:**
   - Tools → Upload Speed → 921600
   - Tools → USB CDC On Boot → "Enabled"
   - Tools → CPU Frequency → 160MHz

4. **Connect XIAO ESP32C6 via USB-C**

5. **Upload:**
   - Sketch → Upload
   - May need to hold BOOT button during upload

### Using PlatformIO

```bash
cd firmware/sender/fuel_sender
pio run --target upload
pio device monitor
```

## Serial Console Menu

After uploading, open Serial Monitor (115200 baud) to access the menu:

```
=== Fuel Sender Initialized ===
1972 VW Superbeetle Fuel Level Monitor
XIAO ESP32C6 + ESP-NOW

ADC initialized (GPIO3, 12-bit, 3.3V range)
Loaded calibration - Empty offset: 0.00 Ω, Full offset: 0.00 Ω
ESP-NOW initialized, CYD display peer added

Setup complete. Type 'menu' for calibration options.
```

**Available Commands:**
- `menu` - Show main menu
- `status` - Display current resistance & fuel percentage
- `cal` - Enter calibration mode
- `reset` - Reset calibration to defaults
- `help` - Show this menu

### Calibration Menu Options

From `cal` command:

```
=== Fuel Sender Calibration Menu ===
  1) Show calibration status
  2) Two-point calibration (RECOMMENDED)
  3) Single-point calibration
  4) Manual offset adjustment
  5) Configure low fuel threshold
  6) Reset calibration to defaults
  7) Exit calibration
```

## Testing

### Pre-Installation Tests

Follow [../../docs/fuel_testing_calibration.md](../../docs/fuel_testing_calibration.md) for:

1. **Hardware Verification** - Check voltage divider circuit
2. **ADC Reading Test** - Verify firmware reads ADC correctly
3. **Fuel Sender Verification** - Check sender resistance (73Ω-10Ω)
4. **End-to-End Test** - Test complete voltage divider chain

### Calibration Procedure

**Recommended: Two-Point Calibration**

1. Open Serial Monitor (115200 baud)
2. Type: `cal`
3. Select: `2` (Two-Point)
4. Position fuel sender at **EMPTY** (float arm down)
5. Press ENTER - system averages 10 readings
6. Position fuel sender at **FULL** (float arm up)
7. Press ENTER - system calculates offsets and saves
8. Verify: `status` command should show 0% at empty, 100% at full

**Accuracy:** ±2% with two-point calibration

### Post-Installation Tests

Follow [../../docs/fuel_testing_calibration.md](../../docs/fuel_testing_calibration.md) for:

5. **CYD Display Reception** - Verify packets received
6. **Long-Term Stability** - 30+ minute runtime test

## Data Packet Structure

The sender transmits this structure via ESP-NOW:

```cpp
typedef struct __attribute__((packed)) {
  uint8_t version;              // Protocol version = 1 (fuel)
  uint32_t timestamp;           // millis() when sent
  uint16_t raw_resistance;      // Resistance in 0.01Ω units
  uint8_t fuel_percent;         // Calculated fuel level (0-100%)
  uint8_t fault_status;         // Bitmask: 0x01=open, 0x02=short, 0x08=low_fuel
  uint16_t sequence_number;     // Packet counter (increments each send)
  uint8_t checksum;             // XOR checksum of all previous bytes
  uint8_t reserved;             // Padding for future use
} FuelDataPacket;  // Total: 14 bytes
```

**Packet Reception on CYD:**
- Version field = 1 identifies fuel packets
- Checksum validates packet integrity
- CYD distinguishes from oil packets (version 3)

See [../../docs/communication-protocol.md](../../docs/communication-protocol.md) for complete protocol documentation.

## Troubleshooting

### ADC Always Reads 0 or 4095 (Wrong Range)

| Symptom | Cause | Solution |
|---------|-------|----------|
| ADC always reads 0V | Fuel sender shorted or missing | Check series resistor is connected to 3.3V |
| ADC always reads 3.3V | Fuel sender open circuit | Verify fuel sender resistance 10-73Ω |
| Jittery readings (±10%) | Electrical noise on GPIO3 | Verify 100nF capacitor across fuel sender |

### Calibration Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| Reads 0% at full tank | Wrong R_series value or sender spec | Verify voltage divider output: 1.9V (empty) to 0.3V (full) |
| Always wrong by fixed % | Calibration offset incorrect | Re-run two-point calibration |
| Offsets reset after power cycle | Preferences write failed | Check ESP32 flash isn't corrupted, try factory reset |

### ESP-NOW Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| Serial shows "Delivery Fail" | Wrong MAC address | Verify CYD_MAC_ADDR in fuel_config.h |
| CYD shows "--%" constantly | Packets not received | Check both devices use WiFi channel 1 |
| Checksum error on CYD | Packet corruption | Reduce WiFi interference, move devices closer |

### Serial Menu Not Responding

| Symptom | Cause | Solution |
|---------|-------|----------|
| No startup message | Firmware not uploading | Check USB connection, verify board selection |
| Menu commands not working | Wrong baud rate | Set Serial Monitor to 115200 baud |
| Garbage characters | ESP32 in bootloop | Try pressing BOOT button during upload |

## Performance Specifications

- **Sample Rate:** 2 Hz (smoothed to 1 Hz transmission)
- **Transmission Latency:** <50ms
- **Temperature Range:** 0°C to +70°C (operating)
- **Resistance Range:** 10Ω to 73Ω
- **Resistance Accuracy:** ±0.5Ω (±0.8%)
- **Fuel Percent Accuracy:** ±2% (with two-point calibration)
- **ESP-NOW Range:** 50-100m line-of-sight
- **Current Draw:** 100-150mA continuous

## Future Enhancements

- [ ] OLED local display with fuel consumption rate
- [ ] Low power sleep mode (wake on configuration change)
- [ ] Battery voltage monitoring in packet
- [ ] Fuel consumption rate calculation on CYD
- [ ] Fuel range estimation (km remaining)
- [ ] Historical fuel level logging to SPIFFS
- [ ] Dynamic MAC configuration via web interface
- [ ] Multi-sender support (multiple fuel senders on one CYD)

## Difference from Oil Sender

This fuel sender is **separate and parallel** to the oil sender:

| Feature | Oil Sender (TempDataPacket v3) | Fuel Sender (FuelDataPacket v1) |
|---------|--------------------------------|----------------------------------|
| Purpose | Temperature & pressure | Fuel level only |
| Sensors | MAX31856 + ADS1115 | Internal ADC only |
| Packet Size | 24 bytes | 14 bytes |
| Update Rate | 1 Hz | 1 Hz |
| Reception | Same CYD, separate callback | Same CYD, separate callback |
| Calibration | Via console menu | Via console menu (two-point) |

**Both senders transmit independently on the same WiFi channel (1) to the same CYD display without interference.**

## References

- [Hardware Wiring Guide](../../hardware/sender/fuel_sender/wiring.md)
- [CYD Display Integration Guide](../../docs/fuel_receiver_integration.md)
- [Testing & Calibration Procedures](../../docs/fuel_testing_calibration.md)
- [Communication Protocol](../../docs/communication-protocol.md)
- [ESP-NOW Setup](../../docs/setup/espnow-setup.md)

## See Also

- [Oil Sender Firmware](../README.md)
- [Display Firmware](../../firmware/display/)
- [Hardware Documentation](../../hardware/sender/)

---

**Last Updated:** January 2026  
**Protocol Version:** 1 (Fuel)  
**Firmware Version:** 1.0  
**Hardware Target:** Seeed Studio XIAO ESP32C6  
**Sensor Target:** 1972 VW Superbeetle Fuel Sender (73Ω-10Ω)
