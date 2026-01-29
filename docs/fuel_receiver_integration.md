# CYD Display Receiver Integration Guide

## Overview

This document explains how to integrate the **new fuel sender packet** into the existing CYD (Cheap Yellow Display) dashboard firmware.

**Key Changes:**
- Add new ESP-NOW callback handler for fuel packets (separate from oil data)
- Parse `FuelDataPacket` (protocol v1) in addition to `TempDataPacket` (protocol v3)
- Display fuel gauge on dashboard UI
- Add low fuel warning indicator

---

## Current Architecture Review

### Existing Receiver (CYD_Speedo_Modern2)

The CYD display currently uses:
- **File:** `CYD_Speedo_Modern2.ino`
- **Existing Packet:** `TempDataPacket` (v3) — contains oil temp, oil pressure, thermocouple data
- **Callback:** `onDataReceive()` — handles oil sender packets
- **Display:** 320x240 TFT with oil pressure gauge, temperature, GPS data

### Communication Architecture

```
┌─ Oil Sender (XIAO ESP32C6) ──→╮
│  Sends TempDataPacket (v3)     │
│  at 1 Hz                        │
│                                 ├── CYD Display (ESP32-S3)
│  ┌─ Fuel Sender (XIAO ESP32C6)─→┤  
│  │  Sends FuelDataPacket (v1)   │   Receives both packets
│  │  at 1 Hz                     │   Updates dashboard UI
│                                 │
└──────────────────────────────→─┘
```

---

## Integration Steps

### Step 1: Include Fuel Packet Header

In `CYD_Speedo_Modern2.ino`, add the fuel packet definition at the top:

```cpp
#include "fuel_data_packet.h"  // Add this new include

// Existing includes:
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_now.h>
// ... etc
```

**Location:** Near the top of CYD_Speedo_Modern2.ino, after other #include statements.

### Step 2: Add Fuel Data Global Variables

After existing packet variables, add fuel-specific globals:

```cpp
// Existing oil sender data
TempDataPacket oil_data;
uint32_t oil_data_last_update = 0;

// NEW: Fuel sender data
FuelDataPacket fuel_data;
uint32_t fuel_data_last_update = 0;
bool fuel_data_received = false;

// Fuel display thresholds
#define FUEL_DATA_TIMEOUT_MS 5000  // Mark fuel as stale if no update for 5 seconds
#define FUEL_LOW_WARNING_PERCENT 15
```

**Location:** In the global variables section of CYD_Speedo_Modern2.ino, after oil_data declarations.

### Step 3: Update ESP-NOW Callback

**IMPORTANT:** The current `onDataReceive()` needs to be updated to handle BOTH packet types.

Replace the existing callback with this improved version:

```cpp
/**
 * ESP-NOW receive callback - handles both oil and fuel packets
 * Packet type is determined by the 'version' field:
 *   version = 3 → TempDataPacket (oil data)
 *   version = 1 → FuelDataPacket (fuel data)
 */
void onDataReceive(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int data_len) {
  if (data_len < 1) return;  // Minimum size: at least version byte
  
  uint8_t packet_version = incomingData[0];
  
  // ===== OIL SENDER PACKET (TempDataPacket, v3) =====
  if (packet_version == 3 && data_len == sizeof(TempDataPacket)) {
    memcpy(&oil_data, incomingData, sizeof(TempDataPacket));
    oil_data_last_update = millis();
    
    // Optional: Log oil data reception
    Serial.print("Oil packet received: Oil Temp=");
    Serial.print(oil_data.oilTemperature);
    Serial.print("°C, Pressure=");
    Serial.print(oil_data.oilPressure);
    Serial.println(" PSI");
  }
  
  // ===== FUEL SENDER PACKET (FuelDataPacket, v1) =====
  else if (packet_version == 1 && data_len == sizeof(FuelDataPacket)) {
    memcpy(&fuel_data, incomingData, sizeof(FuelDataPacket));
    fuel_data_last_update = millis();
    fuel_data_received = true;
    
    // Validate checksum
    if (!fuel_packet_is_valid(&fuel_data)) {
      Serial.println("WARNING: Fuel packet checksum invalid!");
      return;
    }
    
    // Check for fault conditions
    if (fuel_data.fault_status & FUEL_FAULT_OPEN_CIRCUIT) {
      Serial.println("ALERT: Fuel sender open circuit!");
    }
    if (fuel_data.fault_status & FUEL_FAULT_SHORT_CIRCUIT) {
      Serial.println("ALERT: Fuel sender short circuit!");
    }
    if (fuel_data.fault_status & FUEL_FAULT_LOW_FUEL) {
      Serial.println("ALERT: Low fuel level!");
    }
    
    // Optional: Log fuel data reception
    Serial.print("Fuel packet received: Level=");
    Serial.print(fuel_data.fuel_percent);
    Serial.print("%, Resistance=");
    Serial.print(fuel_data.raw_resistance / 100.0, 1);  // Convert from 0.01Ω units
    Serial.print(" Ω, Faults=0x");
    Serial.println(fuel_data.fault_status, HEX);
  }
  
  // Unknown packet version
  else {
    Serial.print("WARNING: Unknown packet version=");
    Serial.print(packet_version);
    Serial.print(" or size mismatch (got ");
    Serial.print(data_len);
    Serial.println(" bytes)");
  }
}
```

**Location:** Replace the existing `onDataReceive()` function. Adapt to your existing oil sender handling code.

### Step 4: Add Fuel Gauge Display Function

Add a new function to render the fuel gauge on the display:

```cpp
/**
 * Draw fuel gauge on TFT display
 * Position: Right side of display, below oil pressure gauge
 * 
 * Shows:
 *   - Fuel level percentage (0-100%)
 *   - Fuel gauge bar (horizontal or vertical)
 *   - Low fuel warning (red background if < 15%)
 *   - Stale data indicator (greyed out if no update in 5s)
 */
void display_fuel_gauge() {
  // Gauge parameters
  const int gauge_x = 160;           // X position (right side)
  const int gauge_y = 160;           // Y position
  const int gauge_width = 150;       // Horizontal bar width
  const int gauge_height = 40;       // Vertical bar height
  
  uint32_t now = millis();
  bool is_stale = (now - fuel_data_last_update) > FUEL_DATA_TIMEOUT_MS;
  bool is_low_fuel = fuel_data.fuel_percent < FUEL_LOW_WARNING_PERCENT;
  
  // === Background color ===
  uint16_t bg_color = TFT_BLACK;
  uint16_t text_color = TFT_WHITE;
  
  if (!fuel_data_received) {
    bg_color = TFT_DARKGREY;
    text_color = TFT_GREY;
  } else if (is_stale) {
    bg_color = TFT_DARKGREY;
    text_color = TFT_GREY;
  } else if (is_low_fuel) {
    bg_color = TFT_RED;
    text_color = TFT_YELLOW;
  }
  
  // === Draw background box ===
  tft.fillRect(gauge_x, gauge_y, gauge_width, gauge_height, bg_color);
  tft.drawRect(gauge_x, gauge_y, gauge_width, gauge_height, TFT_LIGHTGREY);
  
  // === Draw fuel bar (proportional to percentage) ===
  if (fuel_data_received && !is_stale) {
    int fill_width = (gauge_width - 4) * fuel_data.fuel_percent / 100;
    uint16_t bar_color = is_low_fuel ? TFT_RED : TFT_GREEN;
    tft.fillRect(gauge_x + 2, gauge_y + 2, fill_width, gauge_height - 4, bar_color);
  }
  
  // === Draw percentage text ===
  tft.setTextColor(text_color, bg_color);
  tft.setTextSize(2);
  tft.setCursor(gauge_x + 10, gauge_y + 10);
  
  if (!fuel_data_received) {
    tft.print("-- %");
  } else if (is_stale) {
    tft.print("-- %");  // Stale data
  } else {
    tft.print(fuel_data.fuel_percent);
    tft.print(" %");
  }
  
  // === Draw label ===
  tft.setTextSize(1);
  tft.setCursor(gauge_x, gauge_y + gauge_height + 5);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Fuel Tank");
  
  // === Draw fault indicator if present ===
  if (fuel_data_received && fuel_data.fault_status & ~FUEL_FAULT_NONE) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(gauge_x + 100, gauge_y + gauge_height + 5);
    tft.print("FAULT!");
  }
}
```

**Location:** Add as a new function, alongside existing `display_oil_pressure()` and similar functions.

### Step 5: Integrate Fuel Display Into Main Loop

In the display update section of `loop()`, call the fuel gauge function:

```cpp
void loop() {
  // ... existing code ...
  
  // Update TFT display
  if (should_update_display()) {
    tft.fillScreen(TFT_BLACK);
    
    // Existing displays
    display_oil_pressure();
    display_oil_temperature();
    display_gps_data();
    
    // NEW: Add fuel gauge display
    display_fuel_gauge();
    
    // Mark display as updated
    last_display_update = millis();
  }
  
  // ... rest of loop ...
}
```

**Location:** Find the display update section in your existing `loop()` function.

### Step 6: Add Data Staleness Check

Ensure fuel data is marked as stale if no updates arrive for 5 seconds:

```cpp
/**
 * Check if fuel data is current
 * Returns true if fuel data is fresh (received within FUEL_DATA_TIMEOUT_MS)
 */
bool is_fuel_data_fresh() {
  if (!fuel_data_received) return false;
  uint32_t elapsed = millis() - fuel_data_last_update;
  return elapsed < FUEL_DATA_TIMEOUT_MS;
}
```

**Location:** Add as a utility function alongside other data check functions.

---

## Testing the Integration

### 1. Verify Compilation

After making changes, compile the CYD firmware:
```bash
Arduino IDE → Sketch → Export Compiled Binary
```

### 2. Flash to CYD Display

```bash
Arduino IDE → Upload to ESP32-S3 (CYD)
```

### 3. Serial Monitor Debugging

Open Serial Monitor (115200 baud) on CYD to see packet reception:
```
Oil packet received: Oil Temp=85.3°C, Pressure=45.2 PSI
Fuel packet received: Level=75%, Resistance=24.5 Ω, Faults=0x00
Fuel packet received: Level=74%, Resistance=25.3 Ω, Faults=0x00
```

### 4. Visual Verification

On the CYD display, you should see:
- **Oil Pressure Gauge** (existing) — showing PSI
- **Oil Temperature** (existing) — showing °C or °F
- **Fuel Tank Gauge** (new) — showing percentage and filled bar
- **Low Fuel Warning** (new) — red background if fuel < 15%
- **No Signal Indicator** (new) — greyed out if no update for 5s

### 5. Test Fault Conditions

With the fuel sender, test:
- **Normal operation** (fuel between 20-100%) — green bar
- **Low fuel** (fuel < 15%) — red bar + "LOW FUEL" warning
- **Disconnect sender** — gauge shows "--%" and "FAULT!"
- **Power off sender** — gauge greyed out after 5 seconds

---

## Optional Enhancements

### 1. Fuel Consumption Display

Track fuel level over time to estimate consumption:

```cpp
struct FuelHistory {
  uint8_t percent;
  uint32_t timestamp;
};

#define HISTORY_SIZE 60
FuelHistory fuel_history[HISTORY_SIZE];  // Last 60 samples (at 1 Hz = 1 minute)
int history_idx = 0;

// In onDataReceive(), record history
if (fuel_data_received) {
  fuel_history[history_idx].percent = fuel_data.fuel_percent;
  fuel_history[history_idx].timestamp = millis();
  history_idx = (history_idx + 1) % HISTORY_SIZE;
}

// Calculate consumption (percent/minute)
float get_fuel_consumption_rate() {
  if (history_idx < 2) return 0.0;
  int oldest_idx = (history_idx + 1) % HISTORY_SIZE;
  int newest_idx = (history_idx == 0) ? HISTORY_SIZE - 1 : history_idx - 1;
  
  float percent_drop = fuel_history[oldest_idx].percent - fuel_history[newest_idx].percent;
  float time_elapsed_ms = fuel_history[newest_idx].timestamp - fuel_history[oldest_idx].timestamp;
  
  return (percent_drop / time_elapsed_ms) * 60000.0;  // percent/minute
}
```

### 2. Fuel Range Estimation

Estimate driving range based on consumption rate:

```cpp
/**
 * Estimate range (km) based on fuel and consumption
 * Assumes 350 km range per full tank (1972 VW Superbeetle typical)
 */
float estimate_fuel_range_km() {
  float full_tank_range = 350.0;  // km (adjust for your vehicle)
  float consumption_rate = get_fuel_consumption_rate();
  
  if (consumption_rate <= 0) {
    // No consumption data yet, estimate from fuel level alone
    return (fuel_data.fuel_percent / 100.0) * full_tank_range;
  }
  
  // Estimate range based on consumption rate
  float minutes_of_fuel = fuel_data.fuel_percent / consumption_rate;
  return (minutes_of_fuel / 60.0) * 60.0;  // km at 60 km/h (adjust speed as needed)
}
```

### 3. Fuel Gauge Animation

Add smooth needle animation for retro-style gauge:

```cpp
// Store previous fuel level for smooth animation
uint8_t prev_fuel_percent = 0;

void display_fuel_gauge_analog() {
  // Smooth interpolation between previous and current level
  float display_percent = prev_fuel_percent + 
    (fuel_data.fuel_percent - prev_fuel_percent) * 0.1;  // 10% per frame
  
  prev_fuel_percent = display_percent;
  
  // Draw gauge needle pointing to display_percent
  // ... draw gauge arc and needle ...
}
```

---

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| Fuel gauge shows "--%" | Fuel sender not connected or powered off | Check ESP-NOW reception, verify sender firmware is running |
| Gauge stays greyed out | Data timeout (no update in 5s) | Check CYD WiFi channel matches sender (should be 1) |
| Checksum error in serial | Packet corruption | Reduce WiFi interference, move sender closer to CYD |
| Compilation error: `fuel_data_packet.h` not found | Header file not included | Copy fuel_data_packet.h to CYD project folder |
| Gauge shows wrong percentage | Calibration offset incorrect on fuel sender | Re-calibrate fuel sender using serial menu |
| Low fuel warning always on | Thresholds not matching | Verify FUEL_LOW_WARNING_PERCENT matches sender config |

---

## Files to Modify

| File | Change |
|------|--------|
| **CYD_Speedo_Modern2.ino** | Add fuel packet include, update onDataReceive(), add display_fuel_gauge() |
| **fuel_data_packet.h** | Copy from fuel_sender project to CYD project folder |

## Reference

- [fuel_data_packet.h](../../firmware/sender-fuel/fuel_data_packet.h) — Packet structure definition
- [fuel_sender.ino](../../firmware/sender-fuel/fuel_sender.ino) — Sender firmware (for reference)
- [fuel_config.h](../../firmware/sender-fuel/fuel_config.h) — Timing and MAC address config

