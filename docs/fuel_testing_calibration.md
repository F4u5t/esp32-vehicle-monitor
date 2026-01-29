# Fuel Sender Testing & Calibration Procedures

## Overview

This document provides step-by-step procedures for testing and calibrating the **XIAO ESP32C6 Fuel Sender** device before and after installation in the 1972 VW Superbeetle.

---

## Pre-Installation Testing

### Test 1: Hardware Verification

**Objective:** Verify the voltage divider circuit is working correctly

**Equipment Needed:**
- Multimeter (voltmeter)
- XIAO ESP32C6 (not yet programmed)
- 100Ω resistor
- 100nF capacitor
- 3.3V power supply or USB cable

**Procedure:**

1. **Assemble voltage divider circuit (no fuel sender yet)**
   ```
   3.3V ----[100Ω resistor]----+---- GPIO3
                                |
                            [100nF cap]
                                |
   GND ──────────────────────────+---- GND
   ```

2. **Power on XIAO C6** using USB cable

3. **Measure voltage at GPIO3 with multimeter**
   - Should read between 0.8V and 1.6V (midpoint ~1.1V)
   - If not, check resistor is connected to 3.3V rail

4. **Verify 3.3V rail voltage**
   - Measure directly on 3.3V pin
   - Should be exactly 3.3V ±0.1V

5. **Verify resistor values** (if using different resistor)
   - Measure R_series with multimeter (should be 100Ω)
   - Measure capacitor (should be 100nF)

**Expected Results:**
- ✓ GPIO3 voltage in range 0.8V-1.6V
- ✓ 3.3V supply stable
- ✓ No shorts or bridges

---

### Test 2: ADC Reading Verification

**Objective:** Verify the ADC can read the voltage divider

**Equipment Needed:**
- XIAO ESP32C6 with fuel_sender.ino flashed
- Serial monitor (115200 baud)
- USB cable

**Procedure:**

1. **Flash fuel_sender.ino** to XIAO C6 using Arduino IDE
   - Board: Seeeduino XIAO ESP32C6
   - COM Port: Select appropriate port
   - Speed: 921600

2. **Open Serial Monitor** (Tools → Serial Monitor, 115200 baud)

3. **Power on XIAO C6** via USB

4. **Observe startup messages:**
   ```
   === Fuel Sender Initialized ===
   1972 VW Superbeetle Fuel Level Monitor
   XIAO ESP32C6 + ESP-NOW
   
   ADC initialized (GPIO3, 12-bit, 3.3V range)
   Loaded calibration - Empty offset: 0.00 Ω, Full offset: 0.00 Ω, Low fuel threshold: 15%
   ESP-NOW initialized, CYD display peer added
     CYD MAC: 08:D1:F9:2A:0B:BC
   Setup complete. Type 'menu' for calibration options.
   ```

5. **Type 'status'** in serial monitor and press ENTER

6. **Observe current resistance reading:**
   ```
   Current: 50.2 Ω -> 36% | Faults: 0x00 | Seq: 123
   ```

7. **Note the resistance value** (should be reasonable if fuel sender is connected, or high voltage indicates open circuit)

**Expected Results:**
- ✓ Firmware initializes successfully
- ✓ Serial communication at 115200 baud works
- ✓ Status command returns valid resistance reading
- ✓ Sequence counter increments each second

---

### Test 3: Fuel Sender Resistance Verification

**Objective:** Verify the actual fuel sender resistance is in expected range

**Equipment Needed:**
- Multimeter (ohmmeter)
- 1972 VW Superbeetle fuel sender (disconnected from tank)

**Procedure:**

1. **Disconnect fuel sender** from vehicle wiring (remove tank sender unit)

2. **Measure resistance with multimeter** across the two terminals
   - Position at fully empty (float arm down): Should read ~73Ω
   - Position at fully full (float arm up): Should read ~10Ω

3. **Record actual values:**
   - Empty resistance: \_\_\_\_\_ Ω
   - Full resistance: \_\_\_\_\_ Ω
   - Range: \_\_\_\_\_ Ω

4. **Compare to expected (1972 VW spec)**
   - Empty: 73Ω nominal (allow ±5Ω)
   - Full: 10Ω nominal (allow ±2Ω)
   - Range: 63Ω nominal (allow ±7Ω)

**Expected Results:**
- ✓ Resistance in expected range for VW sender
- ✓ Values match between multiple measurements
- ✓ No stuck or intermittent readings

**If Resistance Out of Range:**
- Sender may be faulty or wrong spec
- Check it's actually a 1972 VW sender (not a different year)
- May need to adjust R_series or recalibrate

---

### Test 4: End-to-End Voltage Divider Test

**Objective:** Verify complete voltage divider chain with actual fuel sender

**Equipment Needed:**
- XIAO ESP32C6 (with firmware)
- Voltage divider circuit
- 1972 VW fuel sender
- Multimeter
- Serial monitor (115200 baud)

**Procedure:**

1. **Connect fuel sender** to voltage divider circuit
   - Fuel sender signal → GPIO3 (via 100Ω and capacitor)
   - Fuel sender ground → GND

2. **Measure voltage at GPIO3** with multimeter while moving fuel sender:
   - At empty (73Ω): Should read ~1.9V
   - At full (10Ω): Should read ~0.3V
   - Should vary smoothly as resistance changes

3. **Observe serial monitor:**
   - Type 'status' repeatedly while manually moving fuel sender position
   - Resistance values should change from ~73Ω → ~10Ω

4. **Record readings at 5 positions:**
   
   | Position | Voltage | Resistance (serial) | Expected Percent |
   |----------|---------|-------------------|-----------------|
   | Empty    | 1.9V    | ~73Ω              | ~0%             |
   | 1/4      | 1.5V    | ~55Ω              | ~25%            |
   | 1/2      | 1.1V    | ~40Ω              | ~50%            |
   | 3/4      | 0.7V    | ~25Ω              | ~75%            |
   | Full     | 0.3V    | ~10Ω              | ~100%           |

**Expected Results:**
- ✓ Voltage range 0.3V-1.9V (matches design)
- ✓ Resistance tracks fuel sender movement
- ✓ Percentage calculation appears correct (0-100% range)
- ✓ Smooth linear response as fuel sender moves

---

## Calibration Procedures

### Procedure: Two-Point Calibration (Recommended)

**Objective:** Adjust fuel sender readings to match actual tank levels

**Equipment Needed:**
- XIAO ESP32C6 with fuel_sender.ino
- 1972 VW fuel tank (in vehicle or bench test)
- Serial monitor (115200 baud)
- Knowledge of tank fill level

**Steps:**

1. **Open Serial Monitor** (115200 baud)

2. **Type 'menu'** to show calibration options:
   ```
   --- Fuel Sender Calibration Menu ---
     'status'   - Show current resistance & fuel %
     'cal'      - Enter calibration mode
     'reset'    - Reset calibration to defaults
     'help'     - Show this menu
   ```

3. **Type 'cal'** to enter calibration mode

4. **Select option '2'** for Two-Point Calibration (RECOMMENDED):
   ```
   === Two-Point Calibration ===
   This method uses empty and full tank measurements for best accuracy.
   ```

5. **Empty Tank Calibration:**
   - Position fuel sender at EMPTY (manually move float arm down)
   - Press ENTER when ready
   - System takes 10 readings, calculates average
   - Records empty resistance calibration offset

6. **Full Tank Calibration:**
   - Position fuel sender at FULL (manually move float arm up)
   - Press ENTER when ready
   - System takes 10 readings, calculates average
   - Records full resistance calibration offset

7. **Calibration Complete:**
   ```
   === Calibration Complete ===
   Empty offset: -0.50 Ω
   Full offset: +0.25 Ω
   Calibration saved to flash.
   ```

8. **Verify calibration:**
   - Type 'status' and check that 0% and 100% values match actual tank state
   - Test at intermediate positions (should read 0%, 50%, 100% appropriately)

**Accuracy:** ±2% with two-point calibration

---

### Procedure: Single-Point Calibration

**Objective:** Quick calibration using one known reference point

**Equipment Needed:**
- XIAO ESP32C6 with fuel_sender.ino
- Known tank level (either empty or full)
- Serial monitor (115200 baud)

**Steps:**

1. **Open Serial Monitor** (115200 baud)

2. **Type 'cal'** to enter calibration mode

3. **Select option '3'** for Single-Point Calibration

4. **Choose reference point:**
   ```
   Which reference do you have?
     1) Empty tank (73 Ω nominal)
     2) Full tank (10 Ω nominal)
     3) Custom resistance
   ```

5. **Position fuel sender** at the chosen reference

6. **System calculates offset** from single measurement

7. **Verify:** Test readings at other positions

**Accuracy:** ±5% with single-point calibration

---

### Procedure: Manual Offset Adjustment

**Objective:** Fine-tune existing calibration with precision values

**Steps:**

1. **Enter calibration menu** ('cal')

2. **Select option '4'** for Manual Offset Adjustment

3. **Adjust empty offset:**
   - Current value shown
   - Enter new offset in Ω (positive or negative)
   - Example: If readings are 2% high at empty, enter offset +0.1Ω

4. **Adjust full offset:**
   - Repeat for full tank adjustment
   - Example: If readings are 3% low at full, enter offset -0.15Ω

5. **Verify changes** are saved to flash

**Example Workflow:**
```
Current empty offset: -0.50 Ω
Enter new empty offset: -0.40   ← Adjusted 0.1Ω higher
Empty offset updated to: -0.40 Ω

Current full offset: +0.25 Ω
Enter new full offset: +0.15   ← Adjusted 0.1Ω lower
Full offset updated to: +0.15 Ω
```

---

## Post-Installation Testing

### Test 5: CYD Display Reception

**Objective:** Verify fuel packets are transmitted and received by CYD display

**Equipment Needed:**
- XIAO ESP32C6 fuel sender (running)
- CYD display (running receiver firmware)
- USB cable (for serial monitoring)
- WiFi interference-free location (if possible)

**Procedure:**

1. **Power on CYD display**
   - Should show existing oil pressure/temperature gauges
   - CYD firmware listening for ESP-NOW packets

2. **Power on fuel sender**
   - Should begin transmitting fuel packets at 1 Hz
   - Check serial monitor for transmission messages:
     ```
     Fuel packet received: Level=75%, Resistance=24.5 Ω, Faults=0x00
     ```

3. **Observe CYD display**
   - New fuel gauge should appear (right side, below oil data)
   - Fuel percentage should update every 1 second
   - Bar should fill/empty as you manually move fuel sender

4. **Verify visual indicators:**
   - Normal fuel (>15%): Green bar
   - Low fuel (<15%): Red bar with "LOW FUEL" warning
   - No signal (5s timeout): Greyed out "-- %"

5. **Test fault detection:**
   - Disconnect fuel sender (temporarily)
   - CYD should show FAULT indicator within 1 second
   - Reconnect and observe recovery

**Expected Results:**
- ✓ Fuel gauge visible on CYD display
- ✓ Updates every 1 second
- ✓ Responds to fuel sender position changes
- ✓ Fault detection working
- ✓ Low fuel warning triggers at <15%

---

### Test 6: Long-Term Stability

**Objective:** Verify fuel sender works reliably over extended operation

**Duration:** 30+ minutes continuous operation

**Procedure:**

1. **Run system continuously** with fuel sender transmitting
   - Manually move fuel sender position every 60 seconds
   - Observe for any transmission errors or dropouts

2. **Monitor serial output** for errors:
   - Should see one "Fuel packet" log per second
   - Total of 30+ packets in 30 minutes

3. **Check for data corruption:**
   - Watch for checksum failure messages
   - Note any packet loss (gaps in sequence numbers)

4. **Verify display stability:**
   - CYD gauge should not flicker or freeze
   - Values should be smooth and continuous

5. **Monitor power consumption:**
   - Measure current draw with multimeter
   - Should be 100-150mA continuous

**Expected Results:**
- ✓ No transmission errors in 30+ minutes
- ✓ All packets received with valid checksums
- ✓ CYD display smooth and responsive
- ✓ No watchdog resets or crashes

---

## Troubleshooting Guide

### Problem: ADC Reads Always 0 or 4095

| Symptom | Cause | Fix |
|---------|-------|-----|
| ADC always reads 0V | Fuel sender shorted or R_series not connected | Check R_series is wired to 3.3V pin |
| ADC always reads 4095 (3.3V) | Fuel sender open or missing | Check fuel sender connection and resistance |
| Jittery ADC readings | Noise coupling to GPIO3 | Add 100nF capacitor across fuel sender |

### Problem: Incorrect Fuel Percentage

| Symptom | Cause | Fix |
|---------|-------|-----|
| Reads 0% at full tank | Wrong R_series value or fuel sender spec | Re-verify resistor, fuel sender ohms |
| Reads 100% at empty | Calibration offsets inverted | Run two-point calibration again |
| Always wrong by fixed amount | Calibration offset incorrect | Use manual adjustment to fine-tune |
| Oscillates ±10% | Insufficient smoothing | Check FUEL_SMOOTHING_ALPHA in fuel_config.h (should be 0.2) |

### Problem: CYD Not Receiving Fuel Packets

| Symptom | Cause | Fix |
|---------|-------|-----|
| Fuel gauge shows "--%" | Fuel sender not transmitting or wrong MAC | Check CYD_MAC_ADDR in fuel_config.h |
| Gauge shows stale after 5s | WiFi interference or channel mismatch | Both should use channel 1 (verify in config files) |
| Checksum error messages | Packet corruption on transmission | Move devices closer, reduce WiFi interference |

### Problem: Calibration Not Saving

| Symptom | Cause | Fix |
|---------|-------|-----|
| Offsets reset after power cycle | Preferences not saving to flash | Check Preferences code in fuel_sender.ino |
| Can't enter calibration menu | Serial connection lost | Verify COM port and baud rate (115200) |

---

## Calibration Data Sheet

**Use this form to record calibration values for future reference:**

```
Vehicle: 1972 VW Superbeetle
Date: _______________
Technician: _______________

=== Hardware Verification ===
Resistor value: _______ Ω (Expected: 100Ω)
Capacitor value: _______ nF (Expected: 100nF)
3.3V supply voltage: _______ V (Expected: 3.3V)
GPIO3 idle voltage: _______ V (Expected: 0.8-1.6V)

=== Fuel Sender Specs ===
Measured empty resistance: _______ Ω (Expected: ~73Ω)
Measured full resistance: _______ Ω (Expected: ~10Ω)
Measured range: _______ Ω (Expected: ~63Ω)

=== Calibration Results ===
Empty offset: _______ Ω
Full offset: _______ Ω
Low fuel threshold: _______%

=== Verification Test ===
Reading at empty (0%): ______%
Reading at 1/4 tank: ______%
Reading at 1/2 tank: ______%
Reading at 3/4 tank: ______%
Reading at full (100%): ______%

=== Notes ===
_________________________________________
_________________________________________
_________________________________________
```

---

## Reference Documents

- [fuel_config.h](../firmware/sender-fuel/fuel_config.h) — Pin assignments & timing constants
- [fuel_sender_wiring.md](../hardware/sender/fuel_sender_wiring.md) — Circuit diagram & resistor values
- [cyd_receiver_integration.md](cyd_receiver_integration.md) — Display firmware integration guide

