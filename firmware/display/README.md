# CYD Display Firmware

This folder contains the firmware for the Cheap Yellow Display (CYD) dashboard unit.

## Overview

The CYD display receives oil temperature and pressure data via ESP-NOW from the sender and GPS data via serial from the laptop, displaying everything on a modern dashboard interface.

## Files

- **CYD_Speedo_Modern2.ino** - Main dashboard display firmware
- **Get_MAC_Address.ino** - Utility sketch to find the CYD's MAC address
- **ESP_NOW_SETUP.md** - ESP-NOW configuration guide (if present)

## Hardware

- **Cheap Yellow Display (CYD)**
  - ESP32 (typically ESP32-WROOM-32)
  - 320x240 ILI9341 TFT LCD
  - XPT2046 touch controller
  - USB connector for power and serial communication
  - microSD card slot (optional)

## Features

### Display Interface
- **Modern Dashboard UI** with large, readable fonts
- **Color-coded warnings:**
  - Green: Normal operating range
  - Yellow: Warning thresholds
  - Red: Critical thresholds
- **Real-time Updates:**
  - Oil temperature (from ESP-NOW sender)
  - Oil pressure (from ESP-NOW sender)
  - GPS speed (from laptop serial)
  - Position, heading, altitude
  - Satellite count and fix status

### Communication
- **ESP-NOW Receiver** - Receives oil data from ESP32C6 sender
- **Serial GPS Input** - Receives GPS data from laptop via USB
- **Status Indicators** - Shows connection status for both data sources

### Data Display Sections

**Top Section - GPS Data:**
- Speed (large, prominent display)
- GPS fix status
- Satellite count
- Position (lat/lon)
- Heading/bearing
- Altitude

**Bottom Section - Engine Data:**
- Oil temperature with color coding
- Oil pressure with color coding
- Last update timestamp
- Connection status

## Required Libraries

Install these libraries via Arduino Library Manager:

```
TFT_eSPI (by Bodmer)
esp_now.h (built-in)
WiFi.h (built-in)
```

### TFT_eSPI Configuration

**IMPORTANT:** TFT_eSPI requires configuration for the CYD hardware.

Edit the TFT_eSPI library's `User_Setup.h` file or create `User_Setup_Select.h`:

```cpp
// For Cheap Yellow Display (CYD)
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TOUCH_CS 14  // XPT2046 touch controller
#define SPI_FREQUENCY  40000000
```

Alternatively, many CYD boards work with the built-in setup #42 or #43 in TFT_eSPI.

## Configuration

### Finding Your CYD's MAC Address

Before using the main firmware, you need to find your CYD's MAC address:

1. **Upload Get_MAC_Address.ino:**
   - Open `Get_MAC_Address.ino` in Arduino IDE
   - Select board: "ESP32 Dev Module"
   - Upload to CYD

2. **Open Serial Monitor** (115200 baud)

3. **Note the MAC address** displayed:
   ```
   === ESP32 MAC Address ===
   Station Mode MAC Address:
   98:A3:16:8E:6A:E4
   ```

4. **Update sender firmware** with this MAC address
   - See [../sender/README.md](../sender/README.md)

### GPS Serial Format

The CYD expects GPS data in pipe-delimited format via serial:

```
speed|fixStatus|latitude|longitude|altitude|heading|satellites
```

Example:
```
45.5|3D Fix|40.7128|-74.0060|100.5|270|8
```

See [../../laptop/README.md](../../laptop/README.md) for laptop GPS setup.

## Upload Instructions

### Using Arduino IDE

1. **Install ESP32 board support:**
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "esp32"

2. **Install TFT_eSPI library:**
   - Sketch → Include Library → Manage Libraries
   - Search "TFT_eSPI" by Bodmer
   - Install latest version
   - Configure for CYD (see above)

3. **Select board settings:**
   - Board: "ESP32 Dev Module"
   - Upload Speed: 921600
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB (32Mb)
   - Partition Scheme: Default 4MB with spiffs

4. **Connect CYD via USB**

5. **Select COM port:**
   - Tools → Port → (your CYD's port)

6. **Upload:**
   - Sketch → Upload

### Using PlatformIO

Create `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    bodmer/TFT_eSPI@^2.5.0
monitor_speed = 115200
upload_speed = 921600
```

Then:
```bash
cd firmware/display
pio run --target upload
pio device monitor
```

## Testing

### Basic Functionality Test

1. **Upload firmware** and the display should show the dashboard

2. **Check display initialization:**
   - Display should show dashboard UI
   - Should see "WAITING FOR SENSOR DATA..." if no ESP-NOW data yet
   - Should see "NO GPS DATA" if no GPS serial input yet

3. **Test ESP-NOW reception:**
   - Make sure sender is running and transmitting
   - Oil temp and pressure should appear on bottom of screen
   - Check Serial Monitor for: `ESP-NOW Received from: XX:XX:XX:XX:XX:XX`

4. **Test GPS serial input:**
   - Connect laptop with GPS data feed
   - Speed should appear in large numbers
   - Position and satellite data should update
   - Check Serial Monitor for: `GPS: 45.5|3D Fix|...`

5. **Verify color coding:**
   - Normal readings should be green
   - High temperature should trigger yellow/red warnings
   - Low pressure should trigger warnings

### Serial Monitor Output

Expected output when running:
```
========================================
CYD Dashboard Display
========================================

CYD MAC Address: 98:A3:16:8E:6A:E4
✓ ESP-NOW initialized
✓ Display initialized

Waiting for data...

ESP-NOW Received from: 98:A3:16:8E:66:F0
Oil Temp: 85.3°C, Oil Pressure: 45.2 PSI

GPS: 45.5|3D Fix|40.7128|-74.0060|100.5|270|8
Speed: 45.5, Satellites: 8
```

## Display Layout

```
┌─────────────────────────────────────┐
│          GPS SPEEDOMETER            │
│                                     │
│         65.5 MPH                    │  ← Large speed display
│                                     │
│  Fix: 3D Fix    Sats: 12            │
│  Lat: 40.7128   Lon: -74.0060       │
│  Heading: 270°  Alt: 100m           │
├─────────────────────────────────────┤
│        ENGINE MONITORING            │
│                                     │
│  Oil Temp: 95.5°C  ●               │  ← Color coded
│  Oil Press: 45 PSI ●               │  ← Color coded
│                                     │
│  Last Update: 2s ago                │
└─────────────────────────────────────┘
```

### Color Coding

**Oil Temperature:**
- Green: < 100°C (normal)
- Yellow: 100-120°C (warm)
- Red: > 120°C (hot)

**Oil Pressure:**
- Red: < 10 PSI (dangerously low)
- Yellow: 10-20 PSI (low)
- Green: > 20 PSI (normal)

*Note: Adjust thresholds based on your engine specifications*

## Troubleshooting

### Display Blank or White Screen
- Check TFT_eSPI configuration
- Verify correct pins for CYD hardware
- Try different SPI frequency (20MHz instead of 40MHz)
- Check display power and connections

### No ESP-NOW Data Received
- Verify sender has correct receiver MAC address
- Check sender Serial Monitor for "✓ Delivery Success"
- Ensure both devices are powered on
- Reduce distance between devices
- Check Serial Monitor for ESP-NOW initialization messages

### No GPS Data
- Verify laptop is sending serial data
- Check USB connection between laptop and CYD
- Open Serial Monitor to see if data is arriving
- Verify baud rate matches (typically 115200)
- Check GPS data format is pipe-delimited

### Garbled Display
- TFT_eSPI configuration mismatch
- Wrong display driver selected
- Try reducing SPI frequency
- Check for loose connections

### Touch Not Working
- Touch functionality not currently implemented in this firmware
- XPT2046 touch controller is present but not used
- Future enhancement opportunity

### Serial Monitor Shows Gibberish
- Wrong baud rate - set to 115200
- USB cable quality issue
- Driver problem - install CH340 or CP2102 drivers

## Data Structures

### ESP-NOW Received Data

The display expects to receive this structure:

```cpp
typedef struct {
  uint8_t protocolVersion;
  float headTemp;            // Currently mapped to oil temp
  float oilTemp;             // Oil temperature
  float oilPressure;         // Oil pressure
  uint32_t timestamp;
} SensorData;
```

**Note:** There may be a structure mismatch with the sender. The sender transmits a more complete `TempDataPacket` structure. This is being addressed in documentation updates.

### GPS Serial Data Format

Pipe-delimited ASCII string:
```
speed|fixStatus|latitude|longitude|altitude|heading|satellites
```

Fields:
- **speed:** GPS speed in MPH (float)
- **fixStatus:** "No Fix", "2D Fix", or "3D Fix"
- **latitude:** Decimal degrees (float)
- **longitude:** Decimal degrees (float)
- **altitude:** Meters above sea level (float)
- **heading:** Compass heading in degrees (float, 0-360)
- **satellites:** Number of satellites tracked (integer)

## Customization

### Adjusting Warning Thresholds

Edit the thresholds in the main `.ino` file:

```cpp
// Temperature thresholds (°C)
#define TEMP_NORMAL_MAX     100
#define TEMP_WARNING_MAX    120

// Pressure thresholds (PSI)
#define PRESSURE_CRITICAL   10
#define PRESSURE_WARNING    20
```

### Changing Display Colors

TFT_eSPI color constants:
```cpp
TFT_BLACK, TFT_WHITE, TFT_RED, TFT_GREEN, 
TFT_BLUE, TFT_YELLOW, TFT_ORANGE, TFT_CYAN, 
TFT_MAGENTA, TFT_DARKGREY
```

Or define custom colors:
```cpp
#define CUSTOM_COLOR tft.color565(R, G, B)  // R, G, B: 0-255
```

### Display Rotation

Change screen orientation:
```cpp
tft.setRotation(0);  // 0, 1, 2, or 3 (90° increments)
```

## Performance

- **Display Update Rate:** ~10 FPS (100ms)
- **ESP-NOW Receive Latency:** <50ms
- **Serial GPS Parse Rate:** 1 Hz (depends on data rate)
- **Screen Resolution:** 320x240 pixels
- **Color Depth:** 16-bit (65,536 colors)

## Future Enhancements

- [ ] Implement touchscreen controls
- [ ] Add configuration menu via touch
- [ ] Adjustable warning thresholds via UI
- [ ] Data logging to SD card
- [ ] Graphing of temperature/pressure over time
- [ ] Multiple display themes
- [ ] Trip computer features (distance, avg speed, etc.)
- [ ] Lap timer for track use
- [ ] OTA firmware updates

## See Also

- [Sender Firmware](../sender/README.md)
- [ESP-NOW Setup Guide](../../docs/setup/espnow-setup.md)
- [GPS Serial Protocol](../../docs/gps-serial-protocol.md)
- [Laptop GPS Setup](../../laptop/README.md)
- [Troubleshooting Guide](../../docs/troubleshooting.md)

## CYD Resources

- [CYD Hardware Info](../../hardware/display/cyd-info.md)
- [TFT_eSPI Library Documentation](https://github.com/Bodmer/TFT_eSPI)
- [CYD Community Projects](https://github.com/topics/cheap-yellow-display)

---

**Last Updated:** January 2026  
**Display:** ILI9341 320x240 TFT  
**Compatible with:** ESP32 Dev Module, CYD variants
