# Sender Devices - Overview

This folder contains firmware for two independent sender devices that transmit data wirelessly to the CYD display via ESP-NOW:

1. **Oil Sender** - Temperature and pressure monitoring (Protocol v3)
2. **Fuel Sender** - Fuel tank level monitoring (Protocol v1)

## Sender Comparison

| Feature | Oil Sender | Fuel Sender |
|---------|-----------|-------------|
| **Purpose** | Engine oil metrics | Fuel tank level |
| **Sensors** | Thermocouple (MAX31856) + Pressure (ADS1115) | Resistance (ADC) |
| **Hardware** | XIAO ESP32C6 + MAX31856 + ADS1115 | XIAO ESP32C6 + voltage divider |
| **Data Types** | Temperature (°C), Pressure (PSI) | Fuel percentage (0-100%) |
| **Protocol** | v3 (24 bytes) | v1 (14 bytes) |
| **Update Rate** | 1 Hz | 1 Hz |
| **Firmware** | [README](README.md) | [README](fuel_sender/README.md) |
| **Hardware Guide** | [wiring.md](../../hardware/sender/wiring.md) | [wiring.md](fuel_sender/wiring.md) |
| **Calibration** | Via serial menu (pressure offset) | Via serial menu (two-point) |
| **Local Display** | SSD1306 OLED | Optional SSD1306 OLED |

## Architecture

Both senders transmit **independently** to the same **CYD display**:

```
┌──────────────────────────────────────────────────────────┐
│                                                          │
│  Oil Sender (XIAO ESP32C6)      Fuel Sender (XIAO ESP32C6)
│  └─ TempDataPacket (v3)    ─┐  ┌─ FuelDataPacket (v1)   │
│     24 bytes, 1Hz          │  │  14 bytes, 1Hz          │
│                            │  │                          │
│  ┌─ MAX31856             │  │  ┌─ ADC (GPIO3)           │
│  │ ┌─ K-type TC          │  │  │ ┌─ Fuel sender         │
│  │ └─ Pressure sensor    │  └──┼─┘ (voltage divider)    │
│  └─ ADS1115             │     │                          │
│                         ESP-NOW (WiFi channel 1)         │
│                            ↓                             │
│                      CYD Display                         │
│                    (ESP32-S3 receiver)                   │
│                      - Oil gauges                        │
│                      - Fuel gauge                        │
│                      - GPS data                          │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

**Key Points:**
- Both senders use **same WiFi channel (1)**
- Packets are **distinguished by version field** (oil=3, fuel=1)
- **No interference** - separate MAC addresses and packet types
- **Concurrent transmission** - both can send simultaneously
- **Independent calibration** - each device maintains its own settings

## Quick Start

### If You Only Have the Oil Sender

You already have the original firmware:
- Use [sender.ino](sender.ino) or [sender_arduino.ino](sender_arduino.ino)
- Follow [README.md](README.md) for setup and calibration

### If You Want to Add Fuel Monitoring

New option: Fuel sender for tank level:
- Use [fuel_sender/fuel_sender.ino](fuel_sender/fuel_sender.ino)
- Follow [fuel_sender/README.md](fuel_sender/README.md)
- Integrate display changes from [../../docs/fuel_receiver_integration.md](../../docs/fuel_receiver_integration.md)

### If You Have Both (Recommended Configuration)

**Recommended setup:**
1. **Oil Sender** - Engine bay, mounted on or near engine
   - Transmits temperature & pressure continuously
   - Uses TempDataPacket (v3)
   
2. **Fuel Sender** - Mounted on fuel tank or interior
   - Transmits fuel level continuously
   - Uses FuelDataPacket (v1)
   
3. **CYD Display** - Dashboard
   - Receives both packet types
   - Displays oil + fuel data simultaneously
   - No configuration needed (firmware handles both)

**Benefits of dual-sender setup:**
- ✅ Complete vehicle monitoring (oil + fuel)
- ✅ Independent sensor diagnostics
- ✅ Modular architecture (easy to add more senders)
- ✅ No interference (separate protocols)
- ✅ Future-proof (supports 3+ senders)

## File Organization

```
firmware/sender/
├── README.md                     ← You are here
├── sender.ino                    ← Oil sender (main sketch)
├── sender_arduino.ino            ← Oil sender (alternate)
├── config.h                      ← Oil sender config
├── data_packet.h                 ← Oil packet structure (v3)
├── console_menu.cpp/h            ← Oil calibration menu
├── settings.cpp/h                ← Oil Preferences storage
├── ads1115_config.h              ← Oil pressure ADC config
│
└── fuel_sender/                  ← NEW: Fuel sender
    ├── README.md                 ← Fuel sender guide
    ├── fuel_sender.ino           ← Fuel sender (main sketch)
    ├── fuel_config.h             ← Fuel sender config
    ├── fuel_data_packet.h        ← Fuel packet structure (v1)
    └── fuel_calibration.cpp      ← Fuel calibration menu
```

## Documentation Map

| Purpose | Oil Sender | Fuel Sender |
|---------|-----------|-------------|
| **Firmware** | [README.md](README.md) | [fuel_sender/README.md](fuel_sender/README.md) |
| **Hardware Wiring** | [../../hardware/sender/wiring.md](../../hardware/sender/wiring.md) | [../../hardware/sender/fuel_sender/wiring.md](fuel_sender/wiring.md) |
| **Display Integration** | (see display firmware) | [../../docs/fuel_receiver_integration.md](../../docs/fuel_receiver_integration.md) |
| **Testing & Calibration** | [../../docs/setup/espnow-setup.md](../../docs/setup/espnow-setup.md) | [../../docs/fuel_testing_calibration.md](../../docs/fuel_testing_calibration.md) |
| **Packet Format** | [../../docs/communication-protocol.md](../../docs/communication-protocol.md) | [../../docs/communication-protocol.md](../../docs/communication-protocol.md) |

## Capabilities by Device

### Oil Sender (TempDataPacket v3)

**Data:**
- Oil temperature (°C) with cold junction compensation
- Oil pressure (PSI) with configurable calibration
- Sensor fault detection (open circuit, sensor errors)
- Battery level placeholder (for future use)

**Hardware:**
- MAX31856 K-type thermocouple amplifier
- ADS1115 16-bit ADC for pressure transducer
- SSD1306 OLED local display (optional)

**Calibration:**
- Single point pressure offset adjustment
- Via serial menu at 115200 baud
- Persistent storage in Preferences

**Range:**
- Temperature: -200°C to +1350°C
- Pressure: 0-100 PSI (configurable)
- Latency: <50ms

### Fuel Sender (FuelDataPacket v1)

**Data:**
- Fuel tank level (0-100%)
- Raw resistance value (ohms)
- Fault detection (open/short circuit, low fuel)
- Sequence counter and checksum

**Hardware:**
- Internal ADC (12-bit, GPIO3)
- Voltage divider (100Ω series + 100nF filter)
- SSD1306 OLED local display (optional)

**Calibration:**
- Two-point calibration (recommended, ±2% accurate)
- Single-point and manual offset options
- Via serial menu at 115200 baud
- Persistent storage in Preferences

**Range:**
- Fuel percent: 0-100%
- Resistance: 10Ω-73Ω (VW spec)
- Latency: <50ms
- ADC precision: ±0.8mV per step

## Network Configuration

### ESP-NOW Settings (Both Senders)

**Common Settings:**
```cpp
WiFi Channel: 1
Encryption: Disabled
Max Payload: 250 bytes (both use <30)
Retry: 3 attempts, 50ms delay
```

**Receiver Details:**
- **CYD Display MAC:** `08:D1:F9:2A:0B:BC` (update in firmware)
- **Oil Sender MAC:** `98:A3:16:8E:66:F0` (for reference)
- **Fuel Sender MAC:** (assigned dynamically, no hardcode needed)

### Packet Reception on CYD

CYD firmware handles both packet types in `onDataReceive()`:

```cpp
// Extract version field from first byte
uint8_t packet_version = incomingData[0];

if (packet_version == 3) {
  // Oil sender packet
  TempDataPacket* oil = (TempDataPacket*)incomingData;
  // ... process oil data
}
else if (packet_version == 1) {
  // Fuel sender packet
  FuelDataPacket* fuel = (FuelDataPacket*)incomingData;
  // ... process fuel data
}
```

**CYD Display Changes Required:**
- Add `#include "fuel_data_packet.h"` header
- Update `onDataReceive()` callback (shown above)
- Add `display_fuel_gauge()` function
- Call fuel display in main loop

See [../../docs/fuel_receiver_integration.md](../../docs/fuel_receiver_integration.md) for complete CYD modifications.

## Configuration

### MAC Addresses

Both senders need to know the **receiver's MAC address**:

**Oil Sender (config.h):**
```cpp
uint8_t receiverMAC[] = {0x08, 0xD1, 0xF9, 0x2A, 0x0B, 0xBC};
```

**Fuel Sender (fuel_config.h):**
```cpp
#define CYD_MAC_ADDR {0x08, 0xD1, 0xF9, 0x2A, 0x0B, 0xBC}
```

**To find your CYD's MAC:**
1. Load `Get_MAC_Address.ino` (in display folder)
2. Upload to CYD
3. Open Serial Monitor (115200 baud)
4. Note displayed MAC address
5. Update both sender files

### Timing Configuration

**Oil Sender (config.h):**
```cpp
#define SAMPLE_INTERVAL_MS 500        // Read sensors: 2 Hz
#define TRANSMIT_INTERVAL_MS 1000     // Send packets: 1 Hz
```

**Fuel Sender (fuel_config.h):**
```cpp
#define SAMPLE_INTERVAL_MS 500        // Read ADC: 2 Hz
#define TRANSMIT_INTERVAL_MS 1000     // Send packets: 1 Hz
```

Both synchronized at 1 Hz transmission (500ms offset between samples).

## Testing

### Verify Single Sender (Oil or Fuel)

1. Upload one sender firmware
2. Open Serial Monitor (115200 baud)
3. Type command: `status` or `menu`
4. Should see readings

### Verify Dual Senders

1. Upload both sender firmwares to separate XIAO devices
2. Power on both senders
3. Power on CYD display
4. **Check CYD serial output:**
   ```
   Oil packet received: Oil Temp=85°C, Pressure=45 PSI
   Fuel packet received: Level=75%, Resistance=24.5Ω
   Oil packet received: Oil Temp=85.2°C, Pressure=45.1 PSI
   Fuel packet received: Level=74%, Resistance=25.3Ω
   ```
5. **Check CYD display:**
   - Oil gauges visible (existing)
   - Fuel gauge visible (new)
   - Both updating smoothly

### Long-Term Stability

Run both senders for 30+ minutes:
- No packet loss or corruption
- Checksums passing
- No watchdog resets
- CYD display responsive

## Troubleshooting

### If Oil Sender Works But Fuel Doesn't

| Check | Solution |
|-------|----------|
| Fuel sender powered? | Connect USB cable or power supply |
| Correct COM port? | Select fuel sender port in IDE, upload |
| CYD_MAC_ADDR correct? | Verify in fuel_config.h, must match CYD |
| WiFi channel 1? | Both must use channel 1 (hardcoded) |
| Voltage divider working? | Measure GPIO3: should be 0.3V-1.9V |

### If Both Senders Send But CYD Shows Only One

| Check | Solution |
|-------|----------|
| onDataReceive() updated? | Must handle both v3 and v1 packets |
| fuel_data_packet.h included? | Add header to CYD firmware |
| packet_version check correct? | Verify if statement distinguishes packets |
| display_fuel_gauge() called? | Must call in main loop for fuel to show |

### If Packets Show "Delivery Fail"

| Check | Solution |
|-------|----------|
| CYD powered on? | Verify CYD display is running |
| CYD MAC correct? | Check against actual CYD MAC (use Get_MAC_Address.ino) |
| WiFi interference? | Move devices closer, change location |
| Channel 1? | Both must use ESPNOW_WIFI_CHANNEL 1 |

## Future: Multi-Sender Architecture

This design supports **3+ senders** without modification:

```cpp
// Generic packet version detection
if (packet.version == 3) { /* Oil */ }
else if (packet.version == 1) { /* Fuel */ }
else if (packet.version == 2) { /* Coolant (future) */ }
else if (packet.version == 4) { /* Battery (future) */ }
// etc.
```

Could add senders for:
- Coolant temperature monitor
- Battery voltage monitor  
- Transmission temperature
- Engine speed / RPM
- Each with unique protocol version

## See Also

- [Oil Sender README](README.md) - Detailed oil sender documentation
- [Fuel Sender README](fuel_sender/README.md) - Detailed fuel sender documentation
- [Hardware Documentation](../../hardware/sender/) - Wiring guides and BOMs
- [Display Firmware](../display/) - CYD receiver code
- [Documentation](../../docs/) - Protocol, setup, and guides

---

**Last Updated:** January 2026  
**Oil Protocol Version:** 3  
**Fuel Protocol Version:** 1  
**Architecture:** Dual-sender ESP-NOW to single receiver
