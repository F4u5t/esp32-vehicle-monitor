# Fuel Sender Hardware - 1972 VW Superbeetle

Complete hardware assembly guide for the **fuel tank level monitor** sender device.

## Quick Reference

| Component | Spec | Purpose |
|-----------|------|---------|
| **MCU** | Seeed XIAO ESP32C6 | Main processor |
| **Sensor** | 1972 VW Fuel Sender | 73Ω (empty) → 10Ω (full) |
| **ADC Input** | GPIO3 (12-bit, 3.3V) | Resistance measurement |
| **Series Resistor** | 100Ω 1/4W ±5% | Voltage divider |
| **Filter Cap** | 100nF ceramic | Noise filtering |
| **Voltage Range** | 0.3V - 1.9V | ADC safe input |

## Wiring Diagram

See [wiring.md](wiring.md) for complete wiring diagram, circuit topology, resistor selection, and troubleshooting.

## Voltage Divider Design

**Circuit:**
```
3.3V ────[100Ω]────●──── GPIO3 (ADC)
                   │
              [100nF cap]
                   │
            [Fuel Sender]
            (73Ω-10Ω var.)
                   │
GND ───────────────●──── GND
```

**Voltage Output:**
- At empty (73Ω): 1.91V (0%)
- At midpoint (40Ω): 1.10V (50%)
- At full (10Ω): 0.30V (100%)

**Noise Filter:**
- 100nF ceramic capacitor across fuel sender terminals
- Reduces ADC jitter from EMI sources

## Component Selection

### Series Resistor

**Recommended: 100Ω**
- Optimal ADC range usage (0.3V-1.9V)
- Good voltage divider midpoint (1.1V)
- Available in 1/4W and 1/2W

**Alternatives** (if 100Ω unavailable):
- 47Ω: Wider voltage range but less centered
- 150Ω: Narrower voltage range
- 220Ω: Even narrower, not recommended

Use resistor tolerance ±5% or better for accuracy.

### Fuel Sender Specifications

**Standard 1972 VW Superbeetle:**
- **Empty:** 73Ω ±2Ω
- **Full:** 10Ω ±1Ω
- **Range:** 63Ω
- **Resistance Type:** Variable resistor (float-driven)

**Verify before installation:**
- Measure with multimeter at empty and full positions
- Should match spec within ±3Ω tolerance
- Different model years may vary

## Assembly Checklist

### Components
- [ ] Seeed Studio XIAO ESP32C6
- [ ] 100Ω 1/4W resistor (or selected value)
- [ ] 100nF ceramic capacitor (0.1µF)
- [ ] Connecting wire (22-24 AWG solid or stranded)
- [ ] Breadboard or perfboard (optional, for prototyping)
- [ ] Solder & soldering iron (if building permanestly)

### Tools
- [ ] Multimeter (voltage & resistance measurement)
- [ ] USB-C cable (for programming)
- [ ] Screen or Arduino IDE serial monitor (for calibration)

### Assembly Steps

1. **Voltage Divider Circuit**
   - Solder 100Ω resistor from 3.3V to GPIO3
   - Solder 100nF capacitor from GPIO3 to GND
   - Connect fuel sender between GPIO3 and GND

2. **Verification**
   - Measure voltage at GPIO3 (should be 0.8V-1.6V before fuel sender)
   - After fuel sender connection: 0.3V-1.9V range

3. **Testing**
   - Flash firmware (fuel_sender.ino)
   - Open serial monitor (115200 baud)
   - Type `status` to verify ADC readings
   - Move fuel sender float arm and watch readings change

## Pinout Reference

### Seeed XIAO ESP32C6 Pin Layout

```
        USB-C
         ↓↓↓
    ┌─────────────┐
    │  XIAO ESP32 │
  5V│• • • • • • •│GND
GND │• • • • • • •│3.3V
  D0│GPIO20   GPIO21│D1  (Serial RX/TX)
  D1│GPIO1    GPIO2 │D2  (I2C SDA/SCL)
  D2│GPIO42   GPIO41│D3
  D3│GPIO40   GPIO39│D4
  D4│GPIO38   GPIO37│D5
  D5│GPIO36   GPIO35│D6
  D6│GPIO7    GPIO8 │D7
  D7│GPIO9    GPIO46│D8
  D8│GPIO3    GPIO4 │D9  ← FUEL ADC INPUT
  D9│GPIO5    GPIO11│D10
    └─────────────┘
```

### Key Signal Routing

| Function | Pin | GPIO |
|----------|-----|------|
| Fuel ADC Input | D8 | GPIO3 |
| Power (3.3V) | 3.3V | — |
| Ground | GND | — |
| Serial RX (for menu) | D0 | GPIO20 |
| Serial TX (for menu) | D0 | GPIO21 |

## Testing Procedure

### Pre-Installation Check

1. **Assemble circuit** (no fuel sender yet)
   - Connect 3.3V → 100Ω resistor → GPIO3
   - Connect 100nF cap from GPIO3 to GND
   - Do NOT connect fuel sender yet

2. **Measure voltages**
   ```
   3.3V rail: 3.30V ±0.1V  ✓
   GPIO3 (idle): 1.1V ±0.2V  ✓
   ```

3. **Upload firmware**
   - Flash fuel_sender.ino to XIAO C6
   - Open serial monitor (115200 baud)
   - Should see startup message

4. **Connect fuel sender**
   - Temporarily connect sender between GPIO3 and GND
   - Type `status` in serial monitor
   - Should show resistance value (likely will vary as you move sender)

5. **Verify voltage range**
   ```
   Empty position (73Ω): 1.9V ± 0.1V  ✓
   Full position (10Ω): 0.3V ± 0.1V  ✓
   ```

### Noise Testing

If ADC readings are jittery (±10% fluctuation):

1. **Verify 100nF capacitor** is present and soldered close to GPIO3
2. **Check ground connections** - ensure low-impedance return to GND
3. **Shorten wiring** between circuit and XIAO
4. **Avoid running power lines** near ADC signal wire
5. If still noisy, try **adding a second 100nF** capacitor in parallel

## Integration into Vehicle

### Mounting

**Fuel Sender Location:**
- Should mount at or inside fuel tank
- Keep wiring away from high-temperature areas
- Ensure float arm moves freely without hitting tank walls

**XIAO C6 Device:**
- Mount in engine bay or vehicle interior (away from direct engine heat)
- Keep away from moisture and water splash
- Ensure good WiFi line-of-sight to CYD display

### Power Supply Options

**Option 1: USB Power (Development/Testing)**
- Connect USB-C cable to XIAO
- Provides 5V → regulated to 3.3V internally
- Best for initial setup and calibration

**Option 2: Vehicle 12V (Most Practical)**
- Use 12V to 3.3V buck converter
- Mount near XIAO C6
- Add reverse polarity protection diode
- Enables fuel monitoring when vehicle is running

**Option 3: LiPo Battery (Future Low-Power Mode)**
- 3.7V LiPo → 3.3V regulator
- Requires low-power firmware modifications
- Useful if sender needs isolation from main vehicle power

### Connectors

**Fuel Sender Connection:**
- Use 2-pin crimp connector or bullet connector
- **Signal wire** (Fuel sender meter pin) → GPIO3 divider
- **Ground wire** (Fuel sender common) → GND

**Adding Quick-Disconnect:**
- Allows fuel sender removal for tank servicing
- Use 4.8mm bullet connectors (common in automotive)
- Clearly label: Ground and Signal

## Calibration

After assembly, calibrate the fuel sender:

1. **Connect fuel sender** to voltage divider
2. **Open serial monitor** (115200 baud)
3. **Type:** `cal`
4. **Select:** `2` (Two-Point Calibration)
5. **Position at EMPTY** → Press ENTER
6. **Position at FULL** → Press ENTER
7. **Verify:** `status` shows 0% and 100%

**Accuracy:** ±2% with two-point calibration

See [../../firmware/sender/fuel_sender/README.md](../../firmware/sender/fuel_sender/README.md#calibration-procedure) for detailed calibration steps.

## Troubleshooting

### ADC Readings Wrong

| Issue | Cause | Solution |
|-------|-------|----------|
| Always reads 4095 (3.3V) | Fuel sender open circuit | Check sender resistance with multimeter |
| Always reads 0 | Fuel sender shorted or missing | Verify series resistor connected to 3.3V |
| Jittery (±10%) | Electrical noise | Add/verify 100nF capacitor |
| Range 0-2048 instead of 0-4095 | ADC resolution issue | Check fuel_config.h ADC settings |

### Voltage Divider Output Wrong

| Measurement | Correct Range | If Wrong |
|---|---|---|
| At empty (73Ω) | 1.8V - 2.0V | Check R_series value or VW sender spec |
| At full (10Ω) | 0.2V - 0.4V | Same as above |
| Midpoint (40Ω) | 1.0V - 1.2V | Indicates non-linearity or bad resistor |

### No Serial Communications

| Issue | Cause | Solution |
|-------|-------|----------|
| No startup message | Firmware upload failed | Try re-uploading, hold BOOT button |
| Garbage characters | Wrong baud rate | Set to 115200 baud rate |
| Commands not responding | COM port disconnected | Reconnect USB cable |

## Advanced: Precision Measurement

If you need higher accuracy than ±2%:

### Option 1: Different R_series Value

- Calculate optimal R_series for your specific sender resistance range
- Use the voltage divider formula: V_adc = VCC × R_fuel / (R_series + R_fuel)
- Iterate to find R_series that maximizes ADC resolution

### Option 2: External ADC (ADS1115)

If precision is critical:
- Use ADS1115 16-bit ADC instead of internal 12-bit ADC
- Provides 0.0015V resolution vs 0.8mV
- Requires I2C library modifications to firmware

### Option 3: Three-Point Calibration

For non-linear fuel gauges:
- Calibrate at empty, half, and full
- Firmware could store multiple offsets
- Requires firmware modification

## References

- [Firmware Documentation](../../firmware/sender/fuel_sender/README.md)
- [CYD Display Integration](../../docs/fuel_receiver_integration.md)
- [Complete Wiring Guide](wiring.md)
- [Testing & Calibration Procedures](../../docs/fuel_testing_calibration.md)
- [Communication Protocol](../../docs/communication-protocol.md)

## Bill of Materials

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | Seeed Studio XIAO ESP32C6 | Cortex-M4, WiFi 2.4GHz |
| 1 | 100Ω 1/4W resistor | ±5% tolerance minimum |
| 1 | 100nF ceramic capacitor | 0.1µF, 5mm pitch |
| 2 | Connecting wires | 22-24 AWG, ~30cm each |
| 1 | 1972 VW Fuel Sender | From vehicle |
| 1 | USB-C cable | For programming |
| Optional: 1 | SSD1306 OLED Display | For local fuel display |
| Optional: 1 | 12V→3.3V buck converter | For permanent vehicle integration |

---

**Last Updated:** January 2026  
**Target Vehicle:** 1972 VW Superbeetle  
**Protocol:** ESP-NOW v1  
**Firmware:** fuel_sender.ino v1.0
