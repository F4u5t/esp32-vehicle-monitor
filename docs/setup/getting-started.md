# Getting Started Guide

Quick start guide to get your ESP32 Vehicle Monitor System up and running.

## Overview

This system has three components that work together:
1. **ESP32C6 Sender** - Monitors oil temp/pressure in engine bay
2. **CYD Display** - Shows data on dashboard
3. **Laptop GPS** - Provides speed and position data

## Prerequisites

Before you begin, ensure you have:
- [ ] All hardware components (see [Bill of Materials](../../hardware/bill-of-materials.md))
- [ ] Arduino IDE 2.0+ installed
- [ ] USB cables for programming
- [ ] Basic soldering skills (if headers need soldering)
- [ ] Laptop running Debian 13 or compatible Linux

## Quick Start Steps

### Phase 1: Get MAC Addresses (15 minutes)

ESP-NOW communication requires MAC addresses to be configured.

1. **Find CYD MAC Address:**
   ```
   - Connect CYD to computer via USB
   - Open Arduino IDE
   - Open: firmware/display/Get_MAC_Address.ino
   - Select Board: "ESP32 Dev Module"
   - Upload and open Serial Monitor (115200 baud)
   - Note the MAC address (format: XX:XX:XX:XX:XX:XX)
   ```

2. **Update Sender Code:**
   ```
   - Open: firmware/sender-oil/sender_arduino.ino
   - Find line ~49: uint8_t receiverMAC[] = {...}
   - Update with CYD's MAC address
   - Convert MAC format: 98:A3:16:8E:6A:E4 → {0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4}
   ```

3. **Document MAC Addresses:**
   - Write down both MAC addresses for reference
   - Label physical devices if desired

**Detailed Guide:** [espnow-setup.md](espnow-setup.md)

---

### Phase 2: Upload Firmware (30 minutes)

#### Install Required Libraries

In Arduino IDE:
```
Sketch → Include Library → Manage Libraries

Install:
- Adafruit MAX31856
- Adafruit ADS1X15  
- Adafruit GFX Library
- Adafruit SSD1306
- TFT_eSPI (by Bodmer)
```

#### Configure TFT_eSPI for CYD

The CYD display requires TFT_eSPI configuration:

1. Find TFT_eSPI library folder:
   - On Windows: `Documents\Arduino\libraries\TFT_eSPI`
   - On Linux: `~/Arduino/libraries/TFT_eSPI`

2. Edit `User_Setup_Select.h`:
   - Comment out default setup
   - Uncomment `#include <User_Setups/Setup42_ILI9341_ESP32.h>` (or similar CYD setup)

3. Or check if your CYD works with default settings

**Detailed Guide:** [display-setup.md](display-setup.md)

#### Upload Sender Firmware

```
1. Connect ESP32C6 via USB-C
2. Open: firmware/sender-oil/sender_arduino.ino
3. Select Board: "XIAO_ESP32C6"
4. Select correct Port
5. Click Upload (may need to hold BOOT button)
6. Open Serial Monitor (115200 baud)
7. Verify: "✓ MAX31856 initialized"
8. Verify: "✓ ESP-NOW initialized"
```

#### Upload Display Firmware

```
1. Connect CYD via USB
2. Open: firmware/display/CYD_Speedo_Modern2.ino
3. Select Board: "ESP32 Dev Module"
4. Select correct Port
5. Click Upload
6. Display should show dashboard UI
```

**Detailed Guides:**
- [sender-setup.md](sender-setup.md)
- [display-setup.md](display-setup.md)

---

### Phase 3: Test Communication (15 minutes)

1. **Power Both Devices:**
   - Connect sender to power
   - Connect display to power
   - Keep devices close together initially (~1 meter)

2. **Check Sender Serial Monitor:**
   ```
   Expected output:
   ========================================
   ESP32C6 Temperature Sender (Engine Bay)
   ========================================
   
   Sender MAC Address: 98:A3:16:8E:66:F0
   ✓ ESP-NOW initialized
   ✓ Peer added successfully
   
   Oil Temp: 25.3°C  Oil Pressure: 0.0 PSI
   ✓ Delivery Success
   ```

3. **Check Display:**
   - Bottom section should show oil temp and pressure
   - Values should update every second
   - Should NOT show "WAITING FOR SENSOR DATA..."

4. **If Not Working:**
   - Verify MAC address in sender code
   - Check both devices show "ESP-NOW initialized"
   - See [Troubleshooting Guide](../troubleshooting.md)

---

### Phase 4: Setup GPS (45 minutes)

1. **Install gpsd on Laptop:**
   ```bash
   sudo apt update
   sudo apt install gpsd gpsd-clients python3-gps
   ```

2. **Connect GPS Dongle:**
   ```bash
   # Plug in USB GPS
   # Find device
   ls -l /dev/ttyUSB*
   ```

3. **Configure gpsd:**
   ```bash
   sudo nano /etc/default/gpsd
   
   # Set:
   DEVICES="/dev/ttyUSB0"
   START_DAEMON="true"
   GPSD_OPTIONS="-n"
   
   # Save and restart
   sudo systemctl restart gpsd
   ```

4. **Test GPS Fix:**
   ```bash
   cgps -s
   # Wait for satellite fix (can take 1-5 minutes outdoors)
   ```

5. **Connect CYD to Laptop:**
   - Connect CYD to laptop via USB
   - Note the serial port (likely /dev/ttyUSB1 or /dev/ttyACM1)

6. **Run GPS Forwarder:**
   - Create Python script from [laptop/README.md](../../laptop/README.md)
   - Update SERIAL_PORT in script
   - Run: `python3 gps_forwarder.py`

7. **Verify GPS on Display:**
   - CYD should show speed (0.0 if stationary)
   - Should show GPS position data
   - Satellite count should be visible

**Detailed Guide:** [laptop/README.md](../../laptop/README.md)

---

### Phase 5: Hardware Installation (Vehicle-Specific)

**Safety First:**
- Work on cool engine
- Disconnect battery negative terminal
- Have appropriate mechanical skills
- Follow safety procedures

#### Install Sender in Engine Bay

1. **Mount Sensors:**
   - Install thermocouple in oil system (adapter fitting required)
   - Install pressure sensor in oil gallery
   - Use Teflon tape on NPT threads
   - Verify tight, leak-free connections

2. **Wire Sender Unit:**
   - Follow [hardware/sender/wiring.md](../../hardware/sender/wiring.md)
   - Use weatherproof enclosure
   - Secure all connections
   - Route wires safely away from hot/moving parts

3. **Power Connection:**
   - Connect to switched 12V (ignition-on power)
   - Use inline fuse (2A recommended)
   - Verify voltage regulator if using 5V input

4. **Test Before Final Installation:**
   - Start engine briefly
   - Check for leaks
   - Verify sensor readings
   - Shut down and inspect

#### Install Display

1. **Choose Dashboard Location:**
   - Within driver's view
   - Doesn't obstruct critical gauges
   - USB cable can reach from laptop

2. **Mount Display:**
   - Use adhesive mount or bracket
   - Ensure stable mounting (no vibration)
   - Route USB cable neatly

3. **Connect to Laptop:**
   - USB cable from laptop to CYD
   - Verify GPS data appears

**Detailed Guide:** See your vehicle's service manual for oil system modifications

---

## Initial Calibration

### Pressure Sensor Calibration

1. **Zero Calibration (Engine Off):**
   ```
   - Connect to sender serial monitor
   - With engine off, pressure should read 0-2 PSI
   - If incorrect, adjust PRESSURE_SENSOR_ZERO_VOLTAGE in config.h
   ```

2. **Full Scale Calibration:**
   ```
   - Use mechanical pressure gauge as reference
   - Run engine at operating temperature
   - Compare readings at various RPMs
   - Adjust PRESSURE_SENSOR_FULL_VOLTAGE if needed
   ```

3. **Save Settings:**
   ```
   - Use console menu: "6. Save Settings"
   - Settings persist in flash memory
   ```

**Detailed Guide:** [calibration.md](../calibration.md)

### Temperature Sensor Verification

1. **Cold Junction Check:**
   - Should read ambient temperature (~20-25°C)
   - If way off, check MAX31856 connections

2. **Operating Temperature:**
   - Compare with vehicle's oil temp gauge (if present)
   - Typical oil temp: 80-110°C when warmed up
   - Should stabilize after 10-15 minutes of driving

---

## Testing Checklist

### Bench Testing (Before Installation)
- [ ] CYD shows correct MAC address
- [ ] Sender MAC address updated
- [ ] Sender transmits successfully ("✓ Delivery Success")
- [ ] Display receives ESP-NOW data
- [ ] GPS gets satellite fix
- [ ] CYD receives GPS serial data
- [ ] All displays update in real-time
- [ ] No error messages in serial monitors

### Vehicle Testing
- [ ] Thermocouple installed properly
- [ ] Pressure sensor installed and sealed
- [ ] No oil leaks
- [ ] Sender powered from vehicle
- [ ] Temperature readings reasonable
- [ ] Pressure readings match known values
- [ ] Display stable while driving
- [ ] GPS speed matches speedometer
- [ ] All warnings/alerts work

---

## Normal Operation

### Startup Sequence

1. **Turn on vehicle ignition** (engine doesn't need to be running)
2. **Sender boots** (~3 seconds)
3. **Display boots** and shows dashboard (~5 seconds)
4. **GPS acquires fix** (15-60 seconds if cold start)
5. **All data begins updating**

### Expected Values

**Oil Temperature:**
- Cold engine: 20-40°C (ambient)
- Warming up: 40-80°C
- Operating: 80-110°C
- Hot (hard driving): 110-130°C
- **WARNING:** >130°C may indicate cooling issues

**Oil Pressure:**
- Idle (hot): 10-20 PSI (varies by engine)
- 2000 RPM: 30-50 PSI
- High RPM: 40-70 PSI
- **CRITICAL:** <10 PSI at idle when hot

**GPS Speed:**
- Should match speedometer ±2 MPH
- Updates every second
- Requires 3D fix for accuracy

---

## Troubleshooting Quick Reference

| Problem | Quick Fix |
|---------|-----------|
| Display shows "WAITING FOR SENSOR DATA" | Check sender MAC address, verify sender running |
| No GPS data | Check gpsd running, verify serial connection |
| Temperature reads 0°C | Check thermocouple connections, verify SPI wiring |
| Pressure reads 0 PSI | Check sensor power, verify I2C connections |
| "Delivery Fail" messages | Move devices closer, check receiver MAC |
| Display blank | Check TFT_eSPI configuration, verify power |

**Full Guide:** [troubleshooting.md](../troubleshooting.md)

---

## Next Steps

Now that your system is running:

1. **Monitor for a few drives** to ensure stability
2. **Fine-tune calibration** if needed
3. **Document your specific installation** for future reference
4. **Set up autostart** for GPS forwarder (optional)
5. **Consider enhancements:**
   - Data logging to SD card
   - Custom warning thresholds
   - Additional sensors

---

## Additional Resources

- **[Sender Setup Guide](sender-setup.md)** - Detailed sender configuration
- **[Display Setup Guide](display-setup.md)** - Detailed display configuration
- **[ESP-NOW Setup](espnow-setup.md)** - MAC address configuration
- **[Hardware Wiring](../../hardware/sender/wiring.md)** - Complete wiring diagrams
- **[Laptop GPS Setup](../../laptop/README.md)** - GPS configuration
- **[Calibration Guide](../calibration.md)** - Sensor calibration procedures
- **[Troubleshooting](../troubleshooting.md)** - Common issues and solutions
- **[Communication Protocol](../communication-protocol.md)** - Technical details

---

## Getting Help

If you encounter issues:

1. Check the **[Troubleshooting Guide](../troubleshooting.md)**
2. Review **Serial Monitor** output from both devices
3. Verify **all wiring** against diagrams
4. Check **library versions** are up to date
5. Test **components individually** to isolate problems

---

**Estimated Setup Time:** 2-3 hours (bench testing) + vehicle-specific installation time

**Last Updated:** January 2026

Good luck with your build! 🚗📊
