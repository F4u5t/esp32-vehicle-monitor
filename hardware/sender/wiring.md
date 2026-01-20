# Wiring Guide - ESP32C6 Temperature Monitor

## Overview
This document provides complete wiring instructions for both the sender (ESP32C6) and receiver (ESP32) components of the cylinder head temperature monitoring system.

---

## Sender Wiring: XIAO ESP32C6 → MAX31855 + OLED Display (Engine Bay Unit)

### Required Components
- **Seeed Studio XIAO ESP32C6** development board
- **Adafruit MAX31855** thermocouple amplifier breakout board
- **K-type thermocouple** with standard connector
- **SSD1306 OLED Display** (128x64 pixels, I2C interface) - **for local viewing when working on engine**
- Jumper wires
- Breadboard (optional)

### Pin Connections

**MAX31856 Thermocouple Amplifier (Head Temp - Hardware SPI):**
| MAX31856 Pin | XIAO ESP32C6 Pin | GPIO | Description                       |
|--------------|------------------|------|-----------------------------------|
| VIN          | 5V (See Note)    | -    | Power (Feed 5V if using regulator)|
| GND          | GND              | -    | Ground                            |
| SCK          | D8               | 19   | SPI Clock                         |
| SDI (MOSI)   | **D10**          | **18**| **SPI MOSI (New Connection)**    |
| SDO (MISO)   | D9               | 20   | SPI MISO                          |
| CS           | D3               | 21   | SPI Chip Select                   |

*(Oil Temperature Sensor removed)*

> **IMPORTANT POWER NOTE for MAX31856 Modules:**  
> Like the 31855, if your MAX31856 module has an onboard "662K" regulator, power it with 5V to ensure stable 3.3V operation.

> **IMPORTANT POWER NOTE for MAX31855 Modules:**  
> Most generic MAX31855 modules have an onboard 3.3V voltage regulator (usually a small 3-pin chip labeled 662K).  
> *   If your module has this regulator, **you MUST power it with 5V**. Feeding it 3.3V will cause the regulator to output ~2.5V, which starves the chip and causes unstable readings.  
> *   The data lines (DO, CLK, CS) remain 3.3V logic safe because the chip itself runs at 3.3V.  
> *   **Exception:** Only power with 3.3V if your module specifically says "3.3V Only" or lacks a regulator.

**ADS1115 ADC (Pressure Sensor - I2C):**

| ADS1115 Pin | XIAO ESP32C6 Pin | GPIO | Description |
|-------------|------------------|------|-------------|
| VDD         | 3.3V             | -    | Power |
| GND         | GND              | -    | Ground |
| SCL         | D5               | 23   | I2C Clock (Shared) |
| SDA         | D4               | 22   | I2C Data (Shared) |
| ADDR        | GND              | -    | Address 0x48 |
| A0          | Sensor output    | -    | 0-5V Signal (Scaled) |

**Pressure Sensor Wiring (Voltage Divider Required):**

Since the ADS1115 is powered by 3.3V, its inputs must not exceed ~3.6V. Your 0-100 PSI sensor outputs up to 4.5V.

**Solution: Voltage Divider**
To safely read the 0-5V signal, we use a voltage divider to scale the voltage down.
*   **R1 (Top Resistor)**: 2.2kΩ
*   **R2 (Bottom Resistor)**: 4.7kΩ

**Wiring Steps:**
1.  **Sensor Signal (0-5V)** connects to one leg of **R1 (2.2kΩ)**.
2.  Connect the other leg of **R1** to **ADS1115 Pin A0**.
3.  Also connect **ADS1115 Pin A0** to one leg of **R2 (4.7kΩ)**.
4.  Connect the other leg of **R2** to **GND**.

*Calculation: 4.5V * (4.7 / (2.2 + 4.7)) = ~3.06V (Safe for 3.3V system).*

**SSD1306 OLED Display (I2C):**

| OLED Pin | XIAO ESP32C6 Pin | GPIO | Description |
|----------|------------------|------|-------------|
| VCC      | 3.3V             | -    | Power supply (3.3V recommended) |
| GND      | GND              | -    | Ground |
| SCL      | D5               | 23   | I2C Clock |
| SDA      | D4               | 22   | I2C Data |

### Wiring Diagram (Text)

XIAO ESP32C6              MAX31855 (Head)       MAX31855 (Oil)        ADS1115 (Press)       SSD1306 OLED
┌──────────────┐          ┌────────────┐        ┌────────────┐        ┌────────────┐        ┌──────────────┐
│              │          │            │        │            │        │            │        │ ┌──────────┐ │
│         3.3V ├──────┬───┤ VIN        ├───┬────┤ VIN        ├───┬────┤ VDD        │   ┌────┤ │          │ │
│          GND ├──────┼───┤ GND        ├───┼────┤ GND        ├───┼────┤ GND        │   │ ┌──┤ │  128x64  │ │
│              │      │   │            │   │    │            │   │    │ │          │   │ │  │ │   OLED   │ │
│   D8 (GPIO19)├──────┼───┤ CLK        ├───┼────┤ CLK        │   │    │  Pressure  │   │ │  │ │  Engine  │ │
│   D9 (GPIO20)├──────┼───┤ DO (MISO)  ├───┼────┤ DO (MISO)  │   │    │   Sensor   │   │ │  │ │   View   │ │
│              │      │   │            │   │    │            │   │    │  ┌───┐     │   │ │  │ │          │ │
│   D3 (GPIO21)├──────┼───┤ CS         │   │    │            │   │    │  │ S ├──R1──┼───A0─┤   │ │  │ └──────────┘ │
│   D2 (GPIO2) ├──────┼───┼────────────┼───┴────┤ CS         │   │    │  └───┘      │     │   │ │  │              │
│              │      │   │            │        │            │   │    │             R2    │   │ │  │              │
│   D4 (GPIO22)├──────┼───┼────────────┼────────┼────────────┼───┼────┤ SDA         gnd   │   │ │  │              │
│   D5 (GPIO23)├──────┼───┼────────────┼────────┼────────────┼───┼────┤ SCL        ├───┼─┼──┤ SCL          │
│              │      │   │            │        │            │   │    │            │   │ │  │              │
│              │      └───┼────────────┼────────┼────────────┼───┴────┤ ADDR (GND) │   │ │  │ VCC          │
│              │          │            │        │            │        └────────────┘   │ └──┤ GND          │
│              │          │  T+ ───┐   │        │  T+ ───┐   │                         |    │              │
│              │          │        │ K-Type     │        │ K-Type                      |    │              │
│              │          │  T- ───┘ TC        │  T- ───┘ TC                        |    │              │
└──────────────┘          └────────────┘        └────────────┘                         └────└──────────────┘

### Important Notes
1. **Voltage**: Both MAX31855 and OLED use 3.3V to match ESP32C6 logic levels
2. **Common Ground**: All GND connections must be connected together
3. **Thermocouple Polarity**: K-type thermocouples have polarity - connect the correct terminals to T+ and T-
4. **SPI Pins**: Pins D8 (CLK) and D9 (DO) are used. MOSI is NOT connected.
5. **I2C Pins**: GPIO22 (D4) and GPIO23 (D5) are used for the OLED display
6. **Display Purpose**: The sender display is for local viewing when working on the engine - main display is on dash-mounted receiver

### Thermocouple Connection
- **T+**: Positive thermocouple terminal (typically yellow wire for K-type)
- **T-**: Negative thermocouple terminal (typically red wire for K-type)
- **Note**: Some K-type thermocouples use different color codes - check your specific thermocouple datasheet

---

## Receiver Wiring: ESP32 → SSD1306 OLED Display

### Required Components
- **ESP32** development board (any standard variant)
- **SSD1306 OLED Display** (128x64 pixels, I2C interface)
- Jumper wires
- Breadboard (optional)

### Pin Connections

| OLED Pin | ESP32 Pin | GPIO | Description |
|----------|-----------|------|-------------|
| VCC      | 3.3V      | -    | Power supply (3.3V recommended, 5V also works) |
| GND      | GND       | -    | Ground |
| SCL      | GPIO22    | 22   | I2C Clock |
| SDA      | GPIO21    | 21   | I2C Data |

### Wiring Diagram (Text)

```
ESP32 Dev Board          SSD1306 OLED Display (128x64)
┌──────────────┐         ┌─────────────────────────────┐
│              │         │  ┌───────────────────────┐  │
│         3.3V ├─────────┤  │                       │  │
│          GND ├─────────┤  │    128x64 OLED        │  │
│              │         │  │                       │  │
│  GPIO22 (SCL)├─────────┤  │    I2C Address:       │  │
│  GPIO21 (SDA)├─────────┤  │    0x30               │  │
│              │         │  │                       │  │
│              │         │  └───────────────────────┘  │
│              │         │                             │
│              │         │  VCC  GND  SCL  SDA         │
146: └──────────────┘         └─────────────────────────────┘
```

### Important Notes
1. **I2C Address**: This display uses address 0x30 (configured in code). Most SSD1306 displays use 0x3C or 0x3D
2. **Pull-up Resistors**: Most OLED modules have built-in pull-up resistors on SCL/SDA
3. **Power**: 3.3V is recommended, but most OLED modules accept 5V as well
4. **Default I2C Pins**: GPIO21 (SDA) and GPIO22 (SCL) are the default I2C pins on ESP32

---

## Power Considerations

### Sender (ESP32C6)
- **Power Source**: USB-C (programming) or external 3.3V-5V supply to VIN pin
- **Current Draw**: ~80-150mA typical (ESP32C6 + MAX31855)
- **Battery Operation**: Can be powered by 3.7V LiPo battery (JST connector on XIAO board)

### Receiver (ESP32)
- **Power Source**: Micro-USB (programming) or external 5V supply
- **Current Draw**: ~100-200mA typical (ESP32 + OLED display)
- **Battery Operation**: Can be powered by external battery pack via VIN or 5V pin

---

## Troubleshooting

### Sender Issues

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| "MAX31855 init failed" | Wiring error | Check all SPI connections, verify CS pin |
| No temperature reading | SPI communication issue | Use logic analyzer or oscilloscope to verify SPI signals |
| Erratic readings | Poor thermocouple connection | Ensure tight connections at screw terminals |
| Fault: OPEN CIRCUIT | Thermocouple disconnected | Check thermocouple wiring and connections |
| Fault: SHORT TO GND | Thermocouple short | Check for shorts to ground |
| Fault: SHORT TO VCC | Thermocouple short | Check for shorts to VCC |

### Receiver Issues

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Display blank | Wrong I2C address | Verify 0x30 address, try 0x3C if it doesn't work |
| Display not detected | Wiring error | Check SCL/SDA connections, verify power |
| Garbage on display | I2C communication issue | Check for loose connections, add pull-up resistors if needed |
| "NO SIGNAL" constantly | Sender not transmitting | Check sender Serial output, verify MAC address configuration |

### General Issues

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| No ESP-NOW communication | Different Wi-Fi channels | Ensure both devices use same channel (set in code) |
| Intermittent connection | Interference or distance | Reduce distance, check for Wi-Fi interference |
| Data corruption | Poor power supply | Use quality USB cables, add decoupling capacitors |

---

## Testing Procedure

### Step 1: Test Sender Hardware
1. Connect MAX31855 to ESP32C6 as shown above
2. Flash sender code
3. Open Serial Monitor (115200 baud)
4. Verify "MAX31855 initialized" message
5. Check for temperature readings every second
6. Verify internal temperature is near room temperature (~20-25°C)
7. **Fault Test**: Disconnect thermocouple → should show "FAULT: OPEN"
8. Reconnect thermocouple → fault should clear

### Step 2: Test Receiver Hardware
1. Connect OLED to ESP32 as shown above
2. Flash receiver code
3. Open Serial Monitor (115200 baud)
4. Verify OLED displays splash screen
5. Verify display shows "NO SIGNAL" (sender not running yet)
6. **Copy MAC address** from Serial Monitor - you'll need this!

### Step 3: Configure Sender with Receiver MAC
1. Copy receiver MAC address from Step 2
2. Open `sender/include/config.h`
3. Update `receiverMAC[]` array with the copied MAC address
4. Example: `uint8_t receiverMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};`
5. Re-flash sender code

### Step 4: Test End-to-End Communication
1. Power on both devices
2. Receiver display should update within 1-2 seconds
3. Verify temperature matches sender Serial output
4. Check sequence numbers increment on both devices
5. Verify "Status: OK" on display

### Step 5: Range and Reliability Testing
1. Move devices apart gradually
2. Monitor Serial output for packet loss
3. Typical range: 50-100 meters line-of-sight
4. Through walls: significantly reduced

---

## Safety Warnings

### High Temperature Applications
⚠️ **WARNING**: K-type thermocouples can measure temperatures up to 1350°C!

1. **Cylinder Head Mounting**: Ensure thermocouple is properly secured and won't contact moving parts
2. **Wire Routing**: Route thermocouple wires away from hot engine parts and moving components
3. **Vibration**: Use proper strain relief - engine vibration can damage connections
4. **Grounding**: Ensure thermocouple and electronics have common ground to prevent voltage spikes
5. **Fire Hazard**: Monitor TEMP_CRITICAL_THRESHOLD and implement automatic shutdown if needed

### Electrical Safety
1. **Power Supply**: Use regulated power supplies only
2. **Battery Operation**: Monitor battery voltage - low voltage causes erratic operation
3. **Static Discharge**: Handle ESP32 boards with ESD precautions
4. **Connections**: Double-check all wiring before applying power

---

## Optional Enhancements

### Sender Enhancements
1. **External Antenna**: Add external antenna to ESP32C6 for increased range (requires antenna connector variant)
2. **Battery Monitoring**: Connect battery voltage divider to ADC pin for battery level monitoring
3. **Status LED**: Add LED to spare GPIO for visual transmission indicator
4. **Data Logging**: Add SD card module for local data logging

### Receiver Enhancements
1. **Larger Display**: Use 1.3" or 2.4" OLED/TFT display for better visibility
2. **Buzzer Alert**: Add buzzer for audio alerts on high temperature
3. **Multiple Sensors**: Configure to receive from multiple sender units
4. **Web Interface**: Add web server for remote monitoring via smartphone

---

## Bill of Materials (BOM)

### Sender Components (Engine Bay)
| Item | Qty | Notes |
|------|-----|-------|
| Seeed Studio XIAO ESP32C6 | 1 | Available from Seeed, Adafruit, DigiKey |
| Adafruit MAX31855 Breakout | 2 | Part #269 |
| K-Type Thermocouple | 2 | Choose length based on application |
| SSD1306 OLED Display | 1 | 128x64, I2C, 0.96" or 1.3" - for local engine bay viewing |
| Resistor 2.2kΩ | 1 | For voltage divider (R1) |
| Resistor 4.7kΩ | 2 | For voltage divider (R2) and spare |
| Jumper Wires (M-F) | 10 | For MAX31855 + OLED connections |
| USB-C Cable | 1 | For programming and power |

### Receiver Components (Dash Mounted)
| Item | Qty | Notes |
|------|-----|-------|
| ESP32 Development Board | 1 | Any standard variant (DevKit, NodeMCU, etc.) |
| SSD1306 OLED Display | 1 | 128x64, I2C, address 0x30 - dash mounted display |
| Jumper Wires (M-F) | 4 | Or use breadboard wire |
| Micro-USB Cable | 1 | For programming and power |

### Optional Components
| Item | Qty | Notes |
|------|-----|-------|
| Breadboard | 2 | For prototyping |
| Enclosure | 2 | Project boxes for permanent installation |
| LiPo Battery (3.7V) | 1 | For portable sender |
| Battery Pack | 1 | For portable receiver |
| Heat Shrink Tubing | - | For wire protection |
| Mounting Hardware | - | Depends on application |

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-16 | 1.0 | Initial version (MAX31856) |
| 2026-01-18 | 1.1 | Updated to MAX31855 (No SDI) |

---

For questions or issues, refer to the troubleshooting section or check the Serial Monitor output for detailed diagnostic information.
