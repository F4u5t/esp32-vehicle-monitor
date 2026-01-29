# ESP32 Vehicle Monitor System

A 4-component wireless vehicle monitoring system for classic vehicles, combining **oil temperature/pressure monitoring**, **fuel tank level**, and **GPS speedometer display**. Built with ESP32 microcontrollers using ESP-NOW communication protocol.



## System Overview



This project monitors critical engine parameters and displays them on a dashboard with GPS speed data:



### Components

1. **Oil Sender** (Engine Bay) - XIAO ESP32C6
   - Monitors oil temperature via K-type thermocouple (MAX31856)
   - Monitors oil pressure via 0-100 PSI sensor (ADS1115 ADC)
   - Local OLED display for engine-side viewing
   - Transmits TempDataPacket (Protocol v3) at 1 Hz via ESP-NOW
   - See: [firmware/sender-oil/README.md](firmware/sender-oil/README.md)

2. **Fuel Sender** (Fuel Tank) - XIAO ESP32C6
   - Monitors fuel tank level via variable resistance sensor (1972 VW spec: 73Ω-10Ω)
   - Voltage divider ADC circuit for safe measurement
   - Two-point field calibration via serial menu
   - Fault detection (open/short circuit, low fuel)
   - Transmits FuelDataPacket (Protocol v1) at 1 Hz via ESP-NOW
   - See: [firmware/sender-fuel/README.md](firmware/sender-fuel/README.md)

3. **CYD Display** (Cheap Yellow Display - Dashboard)

   - ESP32 with 320x240 TFT touchscreen

   - Receives oil sensor data (temperature + pressure) via ESP-NOW

   - Receives fuel tank level via ESP-NOW

   - Receives GPS data via serial from laptop

   - Modern dashboard UI displaying all data simultaneously

   - Color-coded warnings for oil pressure, fuel level, and temperature



4. **Laptop GPS** (Data Source)

   - Debian 13 Linux system with U-blox GPS dongle

   - Runs `gpsd` daemon for GPS data processing

   - Sends formatted GPS data to CYD via serial connection

   - Provides: Speed, Position, Heading, Altitude, Satellites



### System Architecture



```
+------------------+
|  ESP32C6 Sender  |  (Engine Bay)
|  - Oil Temp      |
|  - Oil Pressure  |---- ESP-NOW ----|
|  - OLED Display  |    (Wireless)   |
+------------------+                 |
                                     v
+------------------+          +------------------+
|  Laptop + GPS    |          |   CYD Display    |  (Dashboard)
|  - gpsd daemon   |--Serial--|   - TFT Screen   |
|  - U-blox GPS    |  (USB)   |   - ESP32        |
+------------------+          +------------------+
```
```



## Features



### Oil Temperature Monitoring

- K-type thermocouple with MAX31856 precision amplifier

- Temperature range: -200Â°C to +1350Â°C

- Â±0.7Â°C accuracy

- Automatic fault detection (open circuit, short circuit)

- Cold junction compensation



### Oil Pressure Monitoring

- 0-100 PSI sensor with voltage output

- 16-bit ADS1115 ADC for precision reading

- Voltage divider circuit for safe ESP32 interfacing

- Configurable pressure calibration

- Color-coded pressure warnings on display



### Fuel Tank Monitoring

- Variable resistance fuel sender (73Ω-10Ω for 1972 VW spec)

- Percentage display calculation with two-point calibration

- Fault detection: open circuit, short circuit, low fuel warnings

- Color-coded fuel level indicator (green >25%, yellow 15-25%, red <15%)

- Field-calibration via serial menu (no reflashing required)

- Exponential smoothing for stable readings



### GPS Speedometer

- Real-time GPS speed display

- Position tracking (latitude/longitude)

- Heading/bearing indication

- Altitude display

- Satellite count and fix quality

- Serial communication from gpsd



### Wireless Communication

- ESP-NOW protocol for low-latency data transmission (oil + fuel independent streams)

- Reliable point-to-point communication

- No WiFi network required

- ~50-100m range line-of-sight

- Simultaneous dual transmission at 1 Hz without interference

- Automatic retry with delivery confirmation



### Display Features

- Modern dashboard UI with large, readable fonts

- Color-coded warnings (temperature, pressure, and fuel level thresholds)

- Real-time data updates on 320x240 TFT screen

- Connection status indicators for both senders

- Fault diagnostics and indicators



## Hardware Requirements



### ESP32C6 Oil Sender (Engine Bay)

- Seeed Studio XIAO ESP32C6

- Adafruit MAX31856 Thermocouple Amplifier

- K-type thermocouple (appropriate length for installation)

- ADS1115 16-bit ADC module

- 0-100 PSI oil pressure sensor (0.5-4.5V output)

- SSD1306 OLED Display (128x64, I2C)

- Voltage divider resistors (2.2kÎ©, 4.7kÎ©)

- Breadboard or custom PCB

- Wiring and connectors



### ESP32C6 Fuel Sender (Fuel Tank)

- Seeed Studio XIAO ESP32C6

- Variable resistance fuel sender (matched to vehicle; 1972 VW: 73Ω-10Ω range)

- Voltage divider circuit components: 100Ω resistor, 100nF capacitor

- SSD1306 OLED Display (128x64, I2C) - optional

- Breadboard or custom PCB

- Wiring and connectors for fuel tank mounting



### CYD Display (Dashboard)

- Cheap Yellow Display (ESP32 + 320x240 ILI9341 TFT)

- USB cable for power and programming

- USB connection to laptop for GPS data



### Laptop GPS System

- Laptop running Debian 13 (or compatible Linux)

- U-blox GPS USB dongle

- gpsd daemon installed

- USB cable for CYD connection



### Bill of Materials

See [hardware/bill-of-materials.md](hardware/bill-of-materials.md) for complete parts list with links.



## Quick Start



### 1. Get MAC Addresses

```bash

# Upload Get_MAC_Address.ino to CYD to find its MAC address

# Note the MAC address displayed in serial monitor

```



### 2. Configure Oil Sender

```cpp

// Update firmware/sender-oil/sender_arduino.ino with CYD MAC address

uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

```



### 3. Configure Fuel Sender

```cpp

// Update firmware/sender-fuel/fuel_sender.ino with CYD MAC address

uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

// Calibrate fuel sender using serial menu (run from USB)

```



### 4. Upload Firmware

```bash

# Upload firmware to all three devices using Arduino IDE

# Oil Sender: firmware/sender-oil/sender_arduino.ino

# Fuel Sender: firmware/sender-fuel/fuel_sender.ino

# Display: firmware/display/CYD_Speedo_Modern2.ino

```



### 5. Configure Laptop GPS

```bash

# Install and configure gpsd on Debian laptop

# See laptop/README.md for detailed setup

```



### 6. Verify Operation

- Check oil sender serial monitor for delivery success messages

- Check fuel sender serial monitor for delivery success messages

- Check CYD display for oil temp/pressure/fuel readings

- Verify GPS data appears on CYD display



For detailed setup instructions, see [docs/setup/getting-started.md](docs/setup/getting-started.md)



## Project Structure



```

vehicle-monitor/

â”œâ”€â”€ README.md                          # This file

â”œâ”€â”€ LICENSE                            # MIT License

â”œâ”€â”€ .gitignore                         # Git ignore rules

â”‚

â”œâ”€â”€ firmware/                          # All firmware code

â”‚   â”œâ”€â”€ sender/                        # ESP32C6 Sender firmware

â”‚   â”‚   â”œâ”€â”€ sender_arduino.ino         # Main sender code

â”‚   â”‚   â”œâ”€â”€ config.h                   # Pin definitions & settings

â”‚   â”‚   â”œâ”€â”€ data_packet.h              # ESP-NOW data structure

â”‚   â”‚   â”œâ”€â”€ console_menu.cpp/h         # Interactive serial menu

â”‚   â”‚   â”œâ”€â”€ settings.cpp/h             # Settings persistence

â”‚   â”‚   â””â”€â”€ ads1115_config.h           # Pressure sensor config

â”‚   â”‚

â”‚   â””â”€â”€ display/                       # CYD Display firmware

â”‚       â”œâ”€â”€ CYD_Speedo_Modern2.ino     # Main display code

â”‚       â””â”€â”€ Get_MAC_Address.ino        # Utility to find MAC

â”‚

â”œâ”€â”€ hardware/                          # Hardware documentation

â”‚   â”œâ”€â”€ sender/                        # Sender wiring & schematics

â”‚   â”‚   â”œâ”€â”€ wiring.md                  # Connection guide

â”‚   â”‚   â””â”€â”€ parts-list.md              # Component list

â”‚   â”œâ”€â”€ display/                       # Display information

â”‚   â”‚   â””â”€â”€ cyd-info.md                # CYD specifications

â”‚   â””â”€â”€ bill-of-materials.md           # Complete BOM

â”‚

â”œâ”€â”€ laptop/                            # Laptop GPS setup

â”‚   â”œâ”€â”€ README.md                      # Setup guide

â”‚   â””â”€â”€ gps-config.md                  # gpsd configuration

â”‚

â”œâ”€â”€ docs/                              # Documentation

â”‚   â”œâ”€â”€ setup/                         # Setup guides

â”‚   â”‚   â”œâ”€â”€ getting-started.md         # Quick start guide

â”‚   â”‚   â”œâ”€â”€ sender-setup.md            # Sender configuration

â”‚   â”‚   â”œâ”€â”€ display-setup.md           # Display configuration

â”‚   â”‚   â””â”€â”€ espnow-setup.md            # ESP-NOW configuration

â”‚   â”œâ”€â”€ communication-protocol.md      # ESP-NOW protocol v3

â”‚   â”œâ”€â”€ gps-serial-protocol.md         # GPS data format

â”‚   â”œâ”€â”€ calibration.md                 # Sensor calibration

â”‚   â””â”€â”€ troubleshooting.md             # Common issues

â”‚

â””â”€â”€ images/                            # Photos and diagrams

    â””â”€â”€ system-architecture.png

```



## Documentation



- **[Getting Started Guide](docs/setup/getting-started.md)** - Complete setup walkthrough

- **[ESP-NOW Setup](docs/setup/espnow-setup.md)** - Configure wireless communication

- **[Oil Sender Documentation](firmware/sender-oil/README.md)** - Oil sender firmware and configuration

- **[Fuel Sender Documentation](firmware/sender-fuel/README.md)** - Fuel sender firmware and calibration

- **[Laptop GPS Setup](laptop/README.md)** - Configure GPS data source

- **[Communication Protocol](docs/communication-protocol.md)** - ESP-NOW protocol (Protocol v3 for oil, Protocol v1 for fuel)

- **[Fuel Receiver Integration](docs/fuel_receiver_integration.md)** - Details on CYD fuel packet reception

- **[Oil Sender Wiring](hardware/sender-oil/wiring.md)** - Oil sender wiring diagrams

- **[Fuel Sender Wiring](hardware/sender-fuel/wiring.md)** - Fuel sender wiring diagrams

- **[Troubleshooting](docs/troubleshooting.md)** - Common issues and solutions

- **[Fuel Testing & Calibration](docs/fuel_testing_calibration.md)** - Fuel sender calibration procedures



## Current MAC Addresses



**Important:** These MAC addresses are specific to the test devices. Update them for your hardware.



| Device | Role | MAC Address |

|--------|------|-------------|

| ESP32-1 | CYD Display (Receiver) | `98:A3:16:8E:6A:E4` |

| ESP32-2 | Oil Sender | `98:A3:16:8E:66:F0` |

| ESP32-3 | Fuel Sender | (Not yet added) |



See [docs/setup/espnow-setup.md](docs/setup/espnow-setup.md) for instructions on updating MAC addresses for both oil and fuel senders.



## Software Requirements



### ESP32 Firmware

- Arduino IDE 2.0+ or PlatformIO

- ESP32 Board Support Package

- Required Arduino libraries:

  - `Adafruit_MAX31856`

  - `Adafruit_ADS1X15`

  - `Adafruit_SSD1306`

  - `TFT_eSPI` (for CYD)

  - `esp_now.h` (built-in)



### Laptop GPS

- Debian 13 or compatible Linux distribution

- gpsd package

- Python 3.x (if using custom scripts)

- U-blox GPS driver support



## Performance Specifications



- **Oil Temperature Range:** -200Â°C to +1350Â°C (K-type thermocouple)

- **Oil Pressure Range:** 0-100 PSI

- **Sample Rate:** 1 Hz (configurable)

- **ESP-NOW Latency:** <50ms typical

- **Communication Range:** 50-100m line-of-sight

- **Display Update Rate:** 10 FPS

- **GPS Update Rate:** 1 Hz (depends on GPS hardware)

- **Temperature Accuracy:** Â±0.7Â°C (MAX31856)

- **Pressure Resolution:** 16-bit (0.0015 PSI steps with ADS1115)



## Data Protocol



### ESP-NOW Data Packet (Protocol v3)

```cpp

typedef struct __attribute__((packed)) {

  uint8_t version;           // Protocol version = 3

  uint32_t timestamp;        // millis()

  float temperature;         // HEAD temp (not currently used)

  float coldJunction;        // HEAD cold junction

  uint8_t faultStatus;       // HEAD fault status

  float oilTemperature;      // OIL temp (Â°C)

  float oilColdJunction;     // OIL cold junction

  uint8_t oilFaultStatus;    // OIL fault status

  float oilPressure;         // OIL pressure (PSI)

  uint8_t sensorsStatus;     // Sensor status bitmask

  uint16_t sequenceNumber;   // Packet counter

  uint8_t batteryLevel;      // Battery level (future)

  uint8_t checksum;          // XOR checksum

} TempDataPacket;

```



### ESP-NOW Fuel Data Packet (Protocol v1)

```cpp

typedef struct __attribute__((packed)) {

  uint8_t version;           // Protocol version = 1

  uint16_t raw_resistance;   // Raw sensor resistance (Ohms)

  uint8_t fuel_percent;      // Calculated fuel percentage (0-100)

  uint8_t fault_status;      // Fault flags (open, short, low fuel)

  uint32_t timestamp;        // millis()

  uint16_t sequence;         // Packet counter

  uint8_t checksum;          // XOR checksum

} FuelDataPacket;

```



### GPS Serial Data Format

Pipe-delimited ASCII string:

```

speed|fixStatus|latitude|longitude|altitude|heading|satellites

Example: "45.5|3D Fix|40.7128|-74.0060|100.5|270|8"

```



See [docs/communication-protocol.md](docs/communication-protocol.md) for complete protocol documentation.



## Safety Warnings



âš ï¸ **IMPORTANT SAFETY INFORMATION**



- This is a DIY monitoring system - **not a certified safety device**

- Do not rely solely on this system for critical safety decisions

- Always use proper thermocouple mounting techniques away from moving parts

- Route all wiring safely away from hot surfaces and rotating components

- Use appropriate strain relief for engine vibration environments

- Verify sensor readings against known-good reference instruments

- Implement automatic engine protection systems if monitoring critical parameters

- Test thoroughly in safe conditions before relying on the system

- **Use at your own risk**



## Troubleshooting



### ESP-NOW Not Working

- Verify MAC addresses are correct in sender code

- Check both devices show "ESP-NOW initialized" in serial monitor

- Ensure devices are within range (start close together)

- Verify sender shows "âœ“ Delivery Success" in serial output



### No GPS Data

- Check laptop is running gpsd daemon: `systemctl status gpsd`

- Verify GPS has fix: `cgps -s` or `gpsmon`

- Check serial connection between laptop and CYD

- Verify baud rate matches on both ends



### Temperature Reading Issues

- Check thermocouple connections (tight screw terminals)

- Verify SPI wiring to MAX31856

- Check for fault status in serial monitor

- Ensure proper grounding



### Pressure Reading Issues

- Verify voltage divider resistor values

- Check ADS1115 I2C connections (SDA, SCL)

- Calibrate using known pressure reference

- Check pressure sensor power supply



See [docs/troubleshooting.md](docs/troubleshooting.md) for detailed troubleshooting guide.



## Future Enhancements



Potential improvements for future versions:

- [ ] SD card data logging

- [ ] Web interface for configuration

- [ ] Bluetooth connectivity option

- [ ] Multiple sensor support (add more temperature/pressure sensors)

- [ ] Configurable alert thresholds via touchscreen

- [ ] Data graphing on display

- [ ] OTA firmware updates

- [ ] CAN bus integration

- [ ] Cloud data logging

- [ ] Mobile app for remote monitoring



## Contributing



This is a personal project, but suggestions and improvements are welcome! Please feel free to:

- Report issues or bugs

- Suggest features or enhancements

- Share your own implementations or modifications

- Contribute documentation improvements



## License



This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.



## Credits



Built using:

- [Arduino](https://www.arduino.cc/) - Development framework

- [ESP-NOW](https://www.espressif.com/en/products/software/esp-now) - Wireless protocol

- [Adafruit Libraries](https://github.com/adafruit) - Sensor drivers

- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) - Display library

- [gpsd](https://gpsd.gitlab.io/gpsd/) - GPS daemon



## Support



For detailed setup and troubleshooting information:

1. Read the [Getting Started Guide](docs/setup/getting-started.md)

2. Check the [Troubleshooting Guide](docs/troubleshooting.md)

3. Review serial monitor output for diagnostic messages

4. Verify wiring against hardware documentation



## Acknowledgments



Special thanks to:

- The ESP32 community for excellent documentation and libraries

- Adafruit for quality sensor breakout boards and libraries

- The gpsd project for reliable GPS data handling



---



**Project Status:** Active Development  

**Last Updated:** January 2026  

**Version:** 3.0 (Protocol v3 with oil pressure monitoring)



Ready to monitor your vehicle! ðŸš—ðŸ“Š

