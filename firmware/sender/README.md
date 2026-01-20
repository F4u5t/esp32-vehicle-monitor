# ESP32C6 Sender Firmware

This folder contains the firmware for the ESP32C6 sender unit that mounts in the engine bay.

## Overview

The sender unit reads oil temperature and pressure sensors and transmits the data wirelessly via ESP-NOW to the CYD display.

## Files

- **sender_arduino.ino** - Main Arduino sketch
- **config.h** - Pin definitions and configuration settings
- **data_packet.h** - ESP-NOW data packet structure (Protocol v3)
- **console_menu.cpp/h** - Interactive serial console menu
- **settings.cpp/h** - Settings persistence using ESP32 Preferences
- **ads1115_config.h** - ADS1115 ADC configuration for pressure sensor

## Hardware

- Seeed Studio XIAO ESP32C6
- Adafruit MAX31856 Thermocouple Amplifier
- K-type thermocouple
- ADS1115 16-bit ADC module
- 0-100 PSI oil pressure sensor
- SSD1306 OLED Display (128x64, I2C)
- Voltage divider circuit (2.2kΩ + 4.7kΩ resistors)

See [../../hardware/sender/wiring.md](../../hardware/sender/wiring.md) for complete wiring diagrams.

## Features

- **Oil Temperature Monitoring**
  - MAX31856 precision thermocouple amplifier
  - K-type thermocouple support
  - Automatic fault detection
  - Cold junction compensation

- **Oil Pressure Monitoring**
  - 16-bit ADC for high precision
  - Configurable calibration values
  - Voltage divider for safe 3.3V input
  - Pressure range: 0-100 PSI

- **Local Display**
  - SSD1306 OLED shows temperature and pressure
  - Real-time sensor readings
  - Fault status display
  - Transmission status

- **ESP-NOW Communication**
  - Low-latency wireless transmission
  - Automatic retry with delivery confirmation
  - 50-100m range line-of-sight
  - Packet sequencing and checksums

- **Console Menu**
  - Interactive serial menu (115200 baud)
  - View sensor readings
  - Configure calibration
  - Display MAC addresses
  - Save settings to flash

## Required Libraries

Install these libraries via Arduino Library Manager or PlatformIO:

```
Adafruit MAX31856 library
Adafruit ADS1X15
Adafruit GFX Library
Adafruit SSD1306
Wire (built-in)
SPI (built-in)
esp_now.h (built-in ESP32)
WiFi.h (built-in ESP32)
Preferences (built-in ESP32)
```

## Configuration

### Pin Assignments (config.h)

**MAX31856 (SPI):**
- CS: GPIO4 (D3 on XIAO)
- SCK: GPIO19 (D8)
- MISO: GPIO20 (D9)
- MOSI: GPIO21 (D10)

**ADS1115 (I2C):**
- SDA: GPIO6 (D4)
- SCL: GPIO7 (D5)

**OLED Display (I2C):**
- SDA: GPIO6 (D4) - shared with ADS1115
- SCL: GPIO7 (D5) - shared with ADS1115
- Address: 0x3C

### Receiver MAC Address

**IMPORTANT:** You must update the receiver MAC address in `sender_arduino.ino`:

```cpp
// Line ~49 - Update with your CYD display's MAC address
uint8_t receiverMAC[] = {0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4};
```

To find your CYD's MAC address:
1. Upload `Get_MAC_Address.ino` to CYD (in `../display/` folder)
2. Open Serial Monitor (115200 baud)
3. Note the MAC address shown
4. Update the `receiverMAC` array in sender code

See [../../docs/setup/espnow-setup.md](../../docs/setup/espnow-setup.md) for detailed instructions.

### Pressure Sensor Calibration

Default calibration values in `config.h`:

```cpp
#define PRESSURE_SENSOR_ZERO_VOLTAGE  0.5    // Voltage at 0 PSI
#define PRESSURE_SENSOR_FULL_VOLTAGE  4.5    // Voltage at 100 PSI
#define PRESSURE_SENSOR_MAX_PSI       100.0  // Maximum pressure
```

You can adjust these values:
1. Via console menu (serial interface)
2. By editing `config.h` and re-uploading

## Upload Instructions

### Using Arduino IDE

1. Install ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "esp32"

2. Select board:
   - Tools → Board → esp32 → "XIAO_ESP32C6"

3. Configure upload settings:
   - Tools → Upload Speed → 921600
   - Tools → USB CDC On Boot → "Enabled"
   - Tools → CPU Frequency → 160MHz

4. Connect XIAO ESP32C6 via USB-C

5. Select correct COM port:
   - Tools → Port → (select your device)

6. Upload:
   - Sketch → Upload
   - **Note:** May need to hold BOOT button during upload

### Using PlatformIO

```bash
cd firmware/sender
pio run --target upload
pio device monitor
```

## Serial Console Menu

After uploading, open Serial Monitor (115200 baud) to access the console menu:

```
========================================
ESP32C6 Temperature Sender (Engine Bay)
========================================

Console Menu:
1. View Current Readings
2. Display Configuration
3. Calibrate Pressure Sensor
4. Display MAC Addresses
5. Reset Settings
6. Save Settings

Enter command:
```

## Testing

### Basic Functionality Test

1. **Upload firmware** and open Serial Monitor (115200 baud)

2. **Check initialization:**
   ```
   ✓ MAX31856 initialized
   ✓ ADS1115 initialized
   ✓ OLED initialized
   ✓ ESP-NOW initialized
   ✓ Peer added successfully
   ```

3. **Verify sensor readings:**
   - Oil temperature should show reasonable value (or room temp if not installed)
   - Oil pressure should show 0 PSI if sensor not pressurized
   - Cold junction should be ~20-25°C

4. **Check ESP-NOW transmission:**
   - Should see: `✓ Delivery Success` after each transmission
   - If seeing: `✗ Delivery Fail` → Check receiver MAC address

5. **View OLED display:**
   - Should show temperature and pressure readings
   - Transmission status indicator

### Fault Detection Test

1. **Disconnect thermocouple:**
   - Should show "OPEN CIRCUIT" fault
   - Serial output shows fault code

2. **Disconnect pressure sensor:**
   - Pressure reading may show unusual value
   - Check voltage reading in console menu

## Troubleshooting

### "MAX31856 init failed"
- Check SPI wiring (CS, SCK, MISO, MOSI)
- Verify MAX31856 power (3.3V)
- Check solder joints

### "ADS1115 not detected"
- Verify I2C wiring (SDA, SCL)
- Check I2C address (should be 0x48)
- Ensure proper power supply

### "ESP-NOW init failed"
- Restart the device
- Check WiFi is not initialized elsewhere
- Verify ESP32C6 board support is up to date

### "Delivery Fail" Messages
- **Wrong MAC address** - Verify receiver MAC in code
- **Receiver not running** - Ensure CYD is powered and firmware loaded
- **Out of range** - Move devices closer for testing
- **WiFi interference** - Try different location

### Temperature Reading Issues
- Check thermocouple polarity (yellow=negative, red=positive)
- Verify tight screw terminal connections
- Check for proper grounding
- Ensure thermocouple is K-type

### Pressure Reading Issues
- Verify voltage divider resistors (2.2kΩ, 4.7kΩ)
- Check pressure sensor wiring
- Run calibration procedure
- Verify sensor output voltage with multimeter

## Data Packet Structure

The sender transmits this structure via ESP-NOW:

```cpp
typedef struct __attribute__((packed)) {
  uint8_t version;           // Protocol version = 3
  uint32_t timestamp;        // millis() timestamp
  float temperature;         // HEAD temp (reserved/unused)
  float coldJunction;        // HEAD cold junction
  uint8_t faultStatus;       // HEAD fault status
  float oilTemperature;      // OIL temperature (°C)
  float oilColdJunction;     // OIL cold junction
  uint8_t oilFaultStatus;    // OIL fault status
  float oilPressure;         // OIL pressure (PSI)
  uint8_t sensorsStatus;     // Sensor status bitmask
  uint16_t sequenceNumber;   // Packet counter
  uint8_t batteryLevel;      // Battery level (future use)
  uint8_t checksum;          // XOR checksum
} TempDataPacket;
```

See [../../docs/communication-protocol.md](../../docs/communication-protocol.md) for complete protocol documentation.

## Performance

- **Sample Rate:** 1 Hz (configurable in `config.h`)
- **Transmission Latency:** <50ms
- **Temperature Range:** -200°C to +1350°C
- **Temperature Accuracy:** ±0.7°C
- **Pressure Resolution:** 16-bit (0.0015 PSI steps)
- **ESP-NOW Range:** 50-100m line-of-sight

## Future Enhancements

- [ ] Battery voltage monitoring
- [ ] Low power sleep mode
- [ ] Multiple thermocouple support
- [ ] SD card data logging
- [ ] Configurable sample rate via console
- [ ] OTA firmware updates
- [ ] Web interface for configuration

## See Also

- [Display Firmware](../display/README.md)
- [Hardware Wiring Guide](../../hardware/sender/wiring.md)
- [ESP-NOW Setup Guide](../../docs/setup/espnow-setup.md)
- [Communication Protocol](../../docs/communication-protocol.md)
- [Troubleshooting Guide](../../docs/troubleshooting.md)

---

**Last Updated:** January 2026  
**Protocol Version:** 3  
**Firmware Version:** 3.0
