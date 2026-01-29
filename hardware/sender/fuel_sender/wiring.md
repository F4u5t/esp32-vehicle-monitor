# Fuel Sender Hardware Wiring Guide

## Overview
This guide details the hardware setup for the **Seeed Studio XIAO ESP32C6 Fuel Sender** device that reads a 1972 VW Superbeetle fuel tank level.

**Key Specifications:**
- **MCU:** Seeed Studio XIAO ESP32C6 (Cortex-M4, 2.4GHz WiFi)
- **Sensor:** 1972 VW Superbeetle fuel sending unit (73Ω empty → 10Ω full)
- **Communication:** ESP-NOW at 1 Hz
- **Update Interval:** 1000ms (1 packet per second)
- **Target Receiver:** CYD Display (Cheap Yellow Display)

---

## Pinout Reference

### Seeed Studio XIAO ESP32C6 Pins

```
        USB-C
         ↓↓↓
    ┌─────────────┐
    │  XIAO ESP32 │
  5V│• • • • • • •│GND
GND │• • • • • • •│3.3V
  D0│GPIO20   GPIO21│D1  (RX/TX)
  D1│GPIO1    GPIO2 │D2  (I2C SDA/SCL)
  D2│GPIO42   GPIO41│D3
  D3│GPIO40   GPIO39│D4
  D4│GPIO38   GPIO37│D5
  D5│GPIO36   GPIO35│D6
  D6│GPIO7    GPIO8 │D7
  D7│GPIO9    GPIO46│D8
  D8│GPIO3    GPIO4 │D9 (ADC capable)
  D9│GPIO5    GPIO11│D10
    └─────────────┘
```

### Main Signal Connectivity

| Signal | XIAO Pin | GPIO | Notes |
|--------|----------|------|-------|
| **Fuel Sender Input** | D8 | GPIO3 | ADC input (12-bit) |
| **SDA (I2C)** | D1 | GPIO1 | For future OLED display |
| **SCL (I2C)** | D2 | GPIO2 | For future OLED display |
| **RX (Serial)** | D0 | GPIO20 | USB serial for menu |
| **TX (Serial)** | D0 | GPIO21 | USB serial for menu |
| **GND** | Any GND | — | Reference ground |
| **3.3V** | 3.3V | — | Power for circuits |

---

## Voltage Divider Circuit

The fuel sender is a **variable resistor** (fuel_sender_resistance = 73Ω empty, 10Ω full). We measure this using a **voltage divider** to convert it to a voltage that the ADC can read.

### Circuit Topology

```
         3.3V
          │
          ├─── [R_series = 100Ω]
          │
          ├─── ADC_input (GPIO3)
          │
          │ (Fuel Sender)
          │    73Ω (empty) to 10Ω (full)
          │
         GND
```

### Resistor Selection

**R_series = 100 Ω** (chosen for optimal ADC range utilization)

**Rationale:**
- At empty (73Ω fuel sender): V_adc = 3.3V × 73/(100+73) = **1.91V** (~70% of 3.3V)
- At full (10Ω fuel sender): V_adc = 3.3V × 10/(100+10) = **0.30V** (~9% of 3.3V)
- **ADC Range Used:** 0.30V to 1.91V (good utilization of 0-3.3V range)
- **Midpoint:** ~1.1V (centered in ADC range for best linearity)

**Alternative Series Resistor Values** (if you have different resistors available):

| R_series | @ Empty (73Ω) | @ Full (10Ω) | Comment |
|----------|---------------|--------------|---------|
| 47Ω | 2.39V | 0.14V | More range but less midpoint room |
| 100Ω | 1.91V | 0.30V | **RECOMMENDED** |
| 150Ω | 1.54V | 0.38V | Less dynamic range |
| 220Ω | 1.19V | 0.42V | Narrower voltage swing |

### Building the Voltage Divider

**Parts Required:**
- 1× 100Ω resistor (1/4W, ±5% tolerance or better)
- 1× 100nF (0.1µF) ceramic capacitor (noise filtering)
- Solid wire or small perfboard for connections

**Assembly Steps:**

1. **Solder R_series (100Ω) from 3.3V**
   ```
   3.3V ----[100Ω]----+---- GPIO3 (ADC)
   ```

2. **Connect fuel sender from GPIO3 to GND**
   ```
   GPIO3 ----[Fuel Sender 73Ω-10Ω]---- GND
   ```

3. **Add capacitor across fuel sender for noise filtering**
   ```
   GPIO3 ----[100nF]----+---- GND
                        │
              [Fuel Sender]
   ```

4. **Full circuit diagram:**
   ```
   3.3V
    │
    ├─[100Ω R_series]─┬─── GPIO3 (ADC input, ~1-2V)
    │                │
    │            ┌──┴─[100nF]─┐  (noise filter)
    │           │              │
    │       [Fuel Sender]      │
    │        (73Ω-10Ω)         │
    │           │              │
   GND ─────────┴──────────────┴─── GND
   ```

---

## Cable Connections

### VW Fuel Sender Connection

The 1972 VW Superbeetle fuel sender is a **2-wire device:**

| Wire | Connection | Notes |
|------|------------|-------|
| **Sender Signal** | → GPIO3 (via voltage divider) | Varies 73Ω-10Ω based on tank level |
| **Sender Ground** | → GND (from XIAO) | Common return path |

**Connector Options:**
- **Crimp terminals** on fuel gauge harness
- **Bullet connectors** (4.8mm) for quick disconnect
- **Screw terminals** on breakout board (simplest for prototyping)

### Example Cable Assembly

```
From fuel tank gauge sender:
   Sender_Signal ──[100Ω resistor]──┐ → GPIO3
                                     │
                                  [100nF cap] to GND
                                     │
   Sender_Ground ────────────────────┤ → GND
                                     │
                                    GND
```

---

## Optional: SSD1306 OLED Display

For local fuel level display on the sender device (optional):

```
XIAO C6 Pins → SSD1306 OLED (I2C)
  GPIO1 (SDA) → SDA (pin 4)
  GPIO2 (SCL) → SCL (pin 3)
  3.3V → VCC (pin 1)
  GND → GND (pin 2)
```

**I2C Address:** 0x3C (default for SSD1306)

---

## Power Considerations

### Supply Options

**Option 1: USB-C (during development)**
- Connect USB cable to XIAO C6
- Provides 5V → regulates to 3.3V internally
- Ideal for testing and calibration

**Option 2: 3.3V Direct Supply**
- External 3.3V regulated power
- Current draw: ~80-150mA
- Good for vehicle integration

**Option 3: LiPo Battery (future low-power mode)**
- 3.7V LiPo → 3.3V regulator
- Enables wireless fuel monitoring if vehicle is off

### Current Budget
- XIAO C6 MCU: ~50-80mA
- WiFi/ESP-NOW active: +30-50mA
- ADC sampling: <1mA
- **Total:** ~100-150mA continuous

---

## Wiring Checklist

### Electronic Components
- [ ] Seeed Studio XIAO ESP32C6
- [ ] 100Ω resistor (1/4W)
- [ ] 100nF ceramic capacitor
- [ ] Connecting wires (solid 22-24 AWG recommended)
- [ ] Breadboard or perfboard (optional, for prototyping)

### Connections to Verify
- [ ] R_series (100Ω) from 3.3V to GPIO3
- [ ] Fuel sender from GPIO3 to GND (via voltage divider)
- [ ] 100nF capacitor across fuel sender (noise filtering)
- [ ] GND connection between XIAO and voltage divider
- [ ] 3.3V connection verified (use multimeter)

### Testing Before Programming
1. **Measure voltage at GPIO3:**
   - Empty tank (fuel sender ~73Ω): ~1.9V
   - Full tank (fuel sender ~10Ω): ~0.3V
   - If not in range, check R_series value and fuel sender resistance

2. **Test fuel sender directly:**
   - Disconnect from divider
   - Measure resistance with multimeter
   - Verify it's in 10-73Ω range (not shorted or open)

---

## Troubleshooting

| Symptom | Cause | Solution |
|---------|-------|----------|
| ADC always reads 4095 (3.3V) | Fuel sender open circuit or not connected | Check wiring; measure sender resistance |
| ADC always reads 0 (0V) | Fuel sender shorted or R_series not connected | Check resistor is connected to 3.3V |
| Jittery/noisy readings | Missing or bad capacitor | Add 100nF across fuel sender |
| Voltage divider output too low | R_series value too high | Reduce R_series (try 47Ω) |
| Voltage divider output saturated at 3.3V | R_series value too low | Increase R_series (try 150Ω) |
| CYD not receiving packets | WiFi channel mismatch or MAC address wrong | Check ESP-NOW config in firmware |
| Calibration persists after power cycle | Preferences not saving | Check ESP32 flash isn't corrupted |

---

## Next Steps

1. **Assemble the voltage divider circuit** with R_series and capacitor
2. **Flash firmware** to XIAO C6 using Arduino IDE
3. **Connect fuel sender** and verify voltage readings
4. **Calibrate** using serial menu (115200 baud):
   - Position at empty tank (73Ω nominal)
   - Position at full tank (10Ω nominal)
   - System calculates offsets automatically
5. **Verify ESP-NOW transmission** to CYD display
6. **Mount in vehicle** near fuel tank sender

---

## Reference Documents

- [fuel_config.h](../firmware/sender/fuel_sender/fuel_config.h) — Pin assignments and timing
- [fuel_sender.ino](../firmware/sender/fuel_sender/fuel_sender.ino) — Firmware main loop
- [fuel_calibration.cpp](../firmware/sender/fuel_sender/fuel_calibration.cpp) — Calibration routines
- [fuel_data_packet.h](../firmware/sender/fuel_sender/fuel_data_packet.h) — ESP-NOW packet format

