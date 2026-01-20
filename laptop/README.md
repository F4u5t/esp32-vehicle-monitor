# Laptop GPS Setup Guide

This guide explains how to set up the laptop GPS component that provides speed and position data to the CYD display.

## Overview

The laptop component uses:
- A Linux system (Debian 13 recommended)
- U-blox GPS USB dongle
- `gpsd` daemon for GPS data processing
- Serial connection to CYD for data transmission

## System Requirements

### Hardware
- Laptop running Debian 13 (or compatible Linux distribution)
- U-blox GPS USB dongle (or compatible GPS receiver)
- USB cable for connecting to CYD display
- 2 available USB ports minimum

### Software
- Debian 13 or compatible Linux distribution
- gpsd package
- Python 3.x (optional, for custom formatting scripts)
- Serial terminal access

### Compatible GPS Hardware

**Recommended GPS Dongles:**
- **VK-162** - U-blox 7 chipset, ~$20, good Linux support
- **BU-353S4** - SiRF Star IV chipset, ~$30, widely used
- **GlobalSat BU-353-S4** - Reliable, well-supported
- **U-blox NEO-6M/7M/8M** based dongles

**Key Requirements:**
- USB interface
- gpsd compatible
- NMEA output support
- External antenna helpful for vehicle use

---

## Installation Steps

### Step 1: Install gpsd

```bash
# Update package list
sudo apt update

# Install gpsd and related tools
sudo apt install gpsd gpsd-clients python3-gps

# Verify installation
gpsd -V
```

Expected output:
```
gpsd: 3.25 (revision 3.25)
```

### Step 2: Connect GPS Dongle

1. **Plug in USB GPS dongle**

2. **Find the device:**
```bash
# List USB devices
lsusb | grep -i gps

# Find serial device (usually /dev/ttyUSB0 or /dev/ttyACM0)
ls -l /dev/ttyUSB* /dev/ttyACM*
```

3. **Check device permissions:**
```bash
# Add your user to dialout group for serial access
sudo usermod -a -G dialout $USER

# Log out and back in for group change to take effect
```

### Step 3: Configure gpsd

1. **Edit gpsd configuration:**
```bash
sudo nano /etc/default/gpsd
```

2. **Update configuration:**
```bash
# Start daemon automatically
START_DAEMON="true"

# GPS device (adjust to your device)
DEVICES="/dev/ttyUSB0"

# Options
GPSD_OPTIONS="-n"

# Listen on all interfaces (localhost by default)
GPSD_SOCKET="/var/run/gpsd.sock"
```

3. **Save and exit** (Ctrl+X, Y, Enter)

### Step 4: Start gpsd Service

```bash
# Stop any running gpsd instances
sudo killall gpsd

# Start gpsd service
sudo systemctl start gpsd

# Enable autostart on boot
sudo systemctl enable gpsd

# Check status
sudo systemctl status gpsd
```

Expected output:
```
● gpsd.service - GPS (Global Positioning System) Daemon
   Loaded: loaded (/lib/systemd/system/gpsd.service; enabled)
   Active: active (running) since...
```

### Step 5: Verify GPS Reception

1. **Check GPS data with cgps:**
```bash
cgps -s
```

Expected output (when GPS has fix):
```
┌───────────────────────────────────────────┐
│ Time:     2026-01-20T15:30:45.000Z        │
│ Latitude:  40.7128°                       │
│ Longitude: -74.0060°                      │
│ Altitude:  100.5 m                        │
│ Speed:     0.0 mph                        │
│ Track:     0.0°                           │
│ Status:    3D FIX (8 sats)               │
└───────────────────────────────────────────┘
```

2. **Alternative: Use gpsmon:**
```bash
gpsmon
```

3. **Wait for GPS fix:**
- Can take 30 seconds to several minutes on first use
- Requires clear view of sky
- More satellites = better accuracy

---

## GPS Data Format for CYD

The CYD display expects GPS data in this pipe-delimited format via serial:

```
speed|fixStatus|latitude|longitude|altitude|heading|satellites
```

### Field Definitions:

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| speed | float | Speed in MPH | `45.5` |
| fixStatus | string | GPS fix quality | `3D Fix`, `2D Fix`, `No Fix` |
| latitude | float | Latitude in decimal degrees | `40.7128` |
| longitude | float | Longitude in decimal degrees | `-74.0060` |
| altitude | float | Altitude in meters | `100.5` |
| heading | float | Heading/bearing in degrees (0-360) | `270` |
| satellites | int | Number of satellites tracked | `8` |

### Example Output:
```
45.5|3D Fix|40.7128|-74.0060|100.5|270|8
0.0|2D Fix|40.7130|-74.0062|95.2|180|5
55.2|3D Fix|40.7135|-74.0065|105.8|275|10
```

---

## Forwarding GPS Data to CYD

You have several options for sending GPS data to the CYD:

### Option 1: Python Script (Recommended)

Create a Python script to read from gpsd and format for CYD:

```python
#!/usr/bin/env python3
"""
GPS to Serial forwarder for CYD Display
Reads from gpsd and sends formatted data to CYD via serial
"""

import serial
import gpsd
import time
from math import isnan

# Configuration
SERIAL_PORT = '/dev/ttyUSB1'  # Adjust to your CYD port
BAUD_RATE = 115200
UPDATE_RATE = 1  # Hz

def main():
    # Connect to gpsd
    gpsd.connect()
    
    # Connect to CYD serial port
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Connected to CYD on {SERIAL_PORT}")
    
    while True:
        try:
            # Get current GPS data
            packet = gpsd.get_current()
            
            # Extract data
            speed = packet.speed() * 2.23694 if hasattr(packet, 'speed') else 0  # m/s to mph
            lat = packet.lat if hasattr(packet, 'lat') else 0
            lon = packet.lon if hasattr(packet, 'lon') else 0
            alt = packet.alt if hasattr(packet, 'alt') else 0
            track = packet.track if hasattr(packet, 'track') else 0
            
            # Get fix status
            mode = packet.mode
            if mode == 3:
                fix_status = "3D Fix"
            elif mode == 2:
                fix_status = "2D Fix"
            else:
                fix_status = "No Fix"
            
            # Get satellite count
            sats = len(packet.sats) if hasattr(packet, 'sats') else 0
            
            # Format data
            data_string = f"{speed:.1f}|{fix_status}|{lat:.6f}|{lon:.6f}|{alt:.1f}|{track:.0f}|{sats}\n"
            
            # Send to CYD
            ser.write(data_string.encode('utf-8'))
            print(f"Sent: {data_string.strip()}")
            
            # Wait for next update
            time.sleep(1.0 / UPDATE_RATE)
            
        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(1)
    
    ser.close()

if __name__ == "__main__":
    main()
```

**Save as:** `gps_forwarder.py`

**Make executable:**
```bash
chmod +x gps_forwarder.py
```

**Install dependencies:**
```bash
sudo apt install python3-serial python3-gpsd
# or
pip3 install pyserial gpsd-py3
```

**Run:**
```bash
./gps_forwarder.py
```

### Option 2: Systemd Service (Autostart)

Create a systemd service to run the GPS forwarder automatically:

```bash
sudo nano /etc/systemd/system/gps-forwarder.service
```

Add:
```ini
[Unit]
Description=GPS to CYD Serial Forwarder
After=gpsd.service
Requires=gpsd.service

[Service]
Type=simple
User=yourusername
ExecStart=/usr/bin/python3 /home/yourusername/gps_forwarder.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Enable and start:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable gps-forwarder
sudo systemctl start gps-forwarder
sudo systemctl status gps-forwarder
```

### Option 3: Manual Testing with Screen

For testing, you can manually view GPS data and verify format:

```bash
# Read from gpsd
gpspipe -w | grep -E 'TPV|SKY'

# Or use cat to monitor gpsd output
nc localhost 2947
```

---

## Connecting CYD to Laptop

1. **Connect CYD via USB cable**

2. **Find CYD serial port:**
```bash
# Before connecting CYD
ls /dev/ttyUSB* /dev/ttyACM*

# Connect CYD, then check again
ls /dev/ttyUSB* /dev/ttyACM*

# New device is your CYD (usually /dev/ttyUSB1 or /dev/ttyACM1)
```

3. **Test serial connection:**
```bash
# Send test data to CYD
echo "45.5|3D Fix|40.7128|-74.0060|100.5|270|8" > /dev/ttyUSB1

# Monitor CYD serial output
screen /dev/ttyUSB1 115200
```

4. **Verify data reception on CYD:**
- Check CYD display for GPS data
- Open CYD Serial Monitor to see debug output

---

## Troubleshooting

### GPS Not Found

```bash
# Check if GPS is detected
lsusb | grep -i gps

# Check dmesg for USB device info
dmesg | tail -20

# Try different USB ports
```

### gpsd Not Starting

```bash
# Check logs
journalctl -u gpsd -n 50

# Kill any hanging processes
sudo killall gpsd

# Restart service
sudo systemctl restart gpsd
```

### No GPS Fix

**Possible causes:**
- Indoors or obstructed view of sky
- GPS dongle antenna issue
- Need to wait longer (cold start can take 5+ minutes)

**Solutions:**
- Move to location with clear sky view
- Check GPS LED indicator (should blink when searching/fixed)
- Wait patiently for initial fix
- Try `gpsd -D 5 -N -n /dev/ttyUSB0` for debug mode

### CYD Not Receiving Data

```bash
# Check CYD is connected
ls -l /dev/ttyUSB1

# Test serial write
echo "test|3D Fix|0|0|0|0|0" > /dev/ttyUSB1

# Monitor with screen
screen /dev/ttyUSB1 115200

# Check permissions
sudo chmod 666 /dev/ttyUSB1
```

### Data Format Issues

- Verify pipe-delimited format
- Check for extra newlines or spaces
- Ensure float values have decimal points
- Verify CYD firmware expects same format

---

## Advanced Configuration

### Higher Update Rate

Edit Python script:
```python
UPDATE_RATE = 5  # Hz (5 updates per second)
```

Note: GPS hardware typically updates at 1 Hz. Higher rates just repeat last value.

### Different Speed Units

Convert to different units in Python script:

```python
# MPH (default)
speed_mph = packet.speed() * 2.23694

# KM/H
speed_kmh = packet.speed() * 3.6

# Knots
speed_knots = packet.speed() * 1.94384
```

### Logging GPS Data

Add to Python script:
```python
import csv
from datetime import datetime

with open('gps_log.csv', 'a', newline='') as f:
    writer = csv.writer(f)
    writer.writerow([datetime.now(), speed, lat, lon, alt, track])
```

---

## Hardware Recommendations

### GPS Antenna Placement

For best reception in vehicle:
- Mount antenna on roof or dashboard
- Avoid metal obstructions
- Keep away from FM/cellular antennas
- Some GPS dongles have external antenna connectors

### Power Supply

GPS dongle typically powered by laptop USB:
- Check power draw (usually <500mA)
- Use powered USB hub if laptop has weak USB
- Some GPS modules need 5V, verify compatibility

### Cable Management

- Use short, quality USB cables
- Secure cables to avoid disconnection while driving
- Consider right-angle USB adapters for tight spaces

---

## Testing Checklist

- [ ] gpsd installed and running
- [ ] GPS dongle detected (`lsusb`)
- [ ] GPS has fix (check with `cgps -s`)
- [ ] Satellites visible (8+ recommended)
- [ ] CYD connected and detected
- [ ] Python script runs without errors
- [ ] CYD displays GPS speed
- [ ] GPS data updates in real-time
- [ ] Autostart service configured (optional)

---

## See Also

- [CYD Display Firmware](../firmware/display/README.md)
- [GPS Serial Protocol](../docs/gps-serial-protocol.md)
- [Getting Started Guide](../docs/setup/getting-started.md)
- [Troubleshooting Guide](../docs/troubleshooting.md)

---

**Last Updated:** January 2026  
**Tested On:** Debian 13 Bookworm  
**GPS Hardware:** U-blox VK-162
