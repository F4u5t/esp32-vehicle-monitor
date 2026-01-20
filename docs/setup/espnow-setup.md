# ESP-NOW MAC Address Configuration Guide

## Current Status: ⚠️ NOT CONFIGURED - Devices Not Communicating

**Date:** 2026-01-20

---

## Overview

Your ESP-NOW communication requires the **sender** to know the **receiver's MAC address**. Since you've changed test devices, the MAC addresses need to be updated.

---

## Step-by-Step Setup Process

### Step 1: Get CYD Display MAC Address

1. **Connect CYD to computer via USB**

2. **Option A: Use Get_MAC_Address sketch**
   - Open Arduino IDE
   - Open: `ESP_CYD_1/Get_MAC_Address.ino`
   - Select board: "ESP32 Dev Module" (or your CYD board)
   - Upload the sketch
   - Open Serial Monitor (115200 baud)
   - **Copy the MAC address** (format: `XX:XX:XX:XX:XX:XX`)

3. **Option B: Use main CYD sketch**
   - Upload `CYD_Speedo_Modern2.ino`
   - Open Serial Monitor (115200 baud)
   - Look for line: `>>> MAC Address: XX:XX:XX:XX:XX:XX`
   - **Copy this MAC address**

**Example Output:**
```
=== ESP32 MAC Address ===
Station Mode MAC Address:
98:A3:16:8E:6A:E4

Use this MAC address in your sender device's peer configuration!
```

---

### Step 2: Update Sender Configuration

1. **Open sender code**
   - File: `ESP_Oil_Data/sender_arduino/sender_arduino.ino`
   - Find line 49: `uint8_t receiverMAC[] = {0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4};`

2. **Convert MAC address to hex array**
   - If CYD MAC is: `AA:BB:CC:DD:EE:FF`
   - Update to: `uint8_t receiverMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};`

3. **Example:**
   ```cpp
   // OLD (from documentation):
   uint8_t receiverMAC[] = {0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4};
   
   // NEW (your actual CYD MAC):
   uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
   ```

---

### Step 3: Upload and Verify Sender

1. **Connect ESP32C6 sender to computer**

2. **Upload sender code**
   - Open: `ESP_Oil_Data/sender_arduino/sender_arduino.ino`
   - Select board: "XIAO ESP32C6"
   - Upload

3. **Check Serial Monitor (115200 baud)**
   - Look for sender's MAC address
   - Verify receiver MAC is correct
   - Should see: `✓ Peer added successfully`

**Expected Output:**
```
========================================
ESP32C6 Temperature Sender (Engine Bay)
========================================

Sender MAC Address: 98:A3:16:8E:66:F0
✓ ESP-NOW initialized
✓ Peer added successfully
Receiver MAC: AA:BB:CC:DD:EE:FF

Starting temperature monitoring...
```

---

### Step 4: Test Communication

1. **Power both devices**
   - Sender (ESP32C6)
   - Receiver (CYD)

2. **Watch sender Serial Monitor**
   - Should see: `✓ Delivery Success` after each transmission
   - If you see: `✗ Delivery Fail` → MAC address is wrong or receiver not running

3. **Watch receiver Serial Monitor**
   - Should see: `ESP-NOW Received from: XX:XX:XX:XX:XX:XX`
   - Should show oil temp and pressure values

4. **Check CYD display**
   - Bottom panel should show oil temp and pressure
   - Should NOT show "WAITING FOR SENSOR DATA..."

---

## Current MAC Addresses (From Documentation)

**⚠️ These may be outdated - verify with actual devices!**

| Device | Role | MAC Address (Old) | Status |
|--------|------|-------------------|--------|
| ESP32-1 | CYD Display (Receiver) | `98:A3:16:8E:6A:E4` | ❓ Verify |
| ESP32-2 | Oil Sender | `98:A3:16:8E:66:F0` | ❓ Verify |
| ESP32-3 | Spare | `98:A3:16:8E:69:B8` | - |

---

## Troubleshooting

### Problem: "✗ Failed to add peer"

**Cause:** Invalid MAC address format

**Solution:**
- Verify MAC address is exactly 6 bytes
- Check for typos in hex values
- Ensure format: `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}`

---

### Problem: "✗ Delivery Fail" on sender

**Possible Causes:**
1. **Receiver not running** - Make sure CYD is powered and running
2. **Wrong MAC address** - Double-check you copied the correct MAC
3. **Different WiFi channels** - Both should use channel 1 (default)
4. **Too far apart** - Move devices closer for testing
5. **Receiver not initialized** - Check CYD Serial Monitor for ESP-NOW init

**Solutions:**
1. Verify CYD is running and shows MAC address on boot
2. Re-check MAC address in sender code
3. Verify both devices show ESP-NOW initialized successfully
4. Move devices within 1-2 meters for initial testing

---

### Problem: CYD shows "WAITING FOR SENSOR DATA..."

**Cause:** No ESP-NOW packets received

**Solutions:**
1. Check sender is transmitting (Serial Monitor should show "✓ Delivery Success")
2. Verify sender has correct receiver MAC address
3. Make sure both devices are powered on
4. Check sender Serial Monitor for errors

---

### Problem: Data received but values are wrong

**Cause:** Protocol structure mismatch

**Check:**
1. Sender sends `TempDataPacket` (Protocol v3)
2. Receiver expects `SensorData` structure
3. **These may not match!** See DOCUMENTATION_AUDIT.md

**Temporary Fix:**
- Values may be in wrong fields
- Check which fields actually contain oil temp/pressure

---

## Quick Reference: MAC Address Conversion

| MAC String | Hex Array |
|------------|-----------|
| `98:A3:16:8E:6A:E4` | `{0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4}` |
| `98:A3:16:8E:66:F0` | `{0x98, 0xA3, 0x16, 0x8E, 0x66, 0xF0}` |
| `AA:BB:CC:DD:EE:FF` | `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}` |

**Conversion Rule:**
- Each pair of characters becomes one hex byte
- Add `0x` prefix to each byte
- Separate with commas
- Enclose in curly braces `{}`

---

## After Configuration

Once ESP-NOW is working:

1. **Document your actual MAC addresses**
   - Update `EspNOW MAC Addresss.txt` with correct values
   - Add to project README

2. **Test range**
   - Move devices apart gradually
   - Typical range: 50-100m line-of-sight

3. **Verify data accuracy**
   - Compare sender Serial output with CYD display
   - Check oil temp and pressure values match

4. **Update documentation**
   - Record which device is which
   - Label physical devices with their MAC addresses

---

## Next Steps After ESP-NOW Works

Once communication is established:

1. ✅ Verify oil temperature readings are accurate
2. ✅ Verify oil pressure readings are accurate  
3. ✅ Test GPS data from laptop to CYD
4. ✅ Calibrate sensors if needed
5. ✅ Update all documentation with correct MAC addresses
6. ✅ Prepare for GitHub publication

---

## Files to Update After MAC Address Change

1. `sender_arduino.ino` (line 49) - receiverMAC array
2. `EspNOW MAC Addresss.txt` - documentation
3. `CONSOLIDATION_PLAN.md` - update MAC addresses
4. Future README.md - document actual MAC addresses

---

**Last Updated:** 2026-01-20  
**Status:** Awaiting MAC address verification from actual devices
