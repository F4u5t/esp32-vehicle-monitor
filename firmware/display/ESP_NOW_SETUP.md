# ESP-NOW Setup for CYD Speedometer

## Overview
This CYD (Cheap Yellow Display) GPS Speedometer has been configured to receive Oil Temperature and Oil Pressure data via ESP-NOW from your sender device.

## Sender Device Information
- **MAC Address**: 98:A3:16:8E:6A:E4
- **Protocol**: ESP-NOW (peer-to-peer)
- **Data Transmitted**: Oil Temperature (°F) and Oil Pressure (PSI)

## Changes Made

### 1. ESP-NOW Integration
- Added ESP-NOW and WiFi libraries
- Created data structure matching the sender's packet format:
  ```cpp
  typedef struct {
    uint8_t protocolVersion;
    float headTemp;
    float oilTemp;
    float oilPressure;
    uint32_t timestamp;
  } SensorData;
  ```

### 2. Receive Callback
- Implemented `onDataReceive()` callback function
- Automatically updates Oil Temp and Oil Pressure when data is received
- Includes debug output to Serial Monitor

### 3. Display Updates
- **Bottom Panel** now displays:
  - **Left side**: Oil Temperature in °F (orange color)
  - **Right side**: Oil Pressure in PSI (green if ≥10 PSI, yellow if <10 PSI)
- Shows "WAITING FOR SENSOR DATA..." when no data received
- Data is considered stale after 5 seconds without updates

### 4. WiFi Configuration
- Device runs in WiFi Station mode (required for ESP-NOW)
- Does NOT connect to any WiFi network
- Only uses WiFi radio for ESP-NOW communication

## How to Verify

### 1. Upload the Code
Upload the modified `CYD_Speedo_Modern2.ino` to your CYD device.

### 2. Check Serial Monitor
Open Serial Monitor at 115200 baud. You should see:
```
=== CYD GPS Speedometer - Modern Design 2 ===
Init WiFi for ESP-NOW...
MAC Address: [Your CYD MAC Address]
Init ESP-NOW...
ESP-NOW initialized successfully
ESP-NOW receive callback registered
Waiting for data from sender: 98:A3:16:8E:6A:E4
```

### 3. Start Sender Device
Power on your sender device (98:A3:16:8E:6A:E4).

### 4. Monitor Data Reception
When data is received, you'll see in Serial Monitor:
```
ESP-NOW Received - Oil Temp: 185.5°F, Oil Pressure: 45.2 PSI
```

### 5. Check Display
The bottom panel should show:
- Oil temperature on the left
- Oil pressure on the right (color-coded)

## Troubleshooting

### No Data Received
1. Verify sender device is powered on and transmitting
2. Check that sender has this CYD's MAC address in its peer list
3. Ensure both devices are using the same WiFi channel
4. Verify the data structure matches between sender and receiver

### Display Shows "WAITING FOR SENSOR DATA..."
- Normal on startup before first packet received
- Appears if no data received for 5+ seconds
- Check sender device is running and transmitting

### Incorrect Values
- Verify the sender's data structure matches exactly
- Check that sender is sending data in the correct order
- Review Serial Monitor debug output

## Data Structure Compatibility

**IMPORTANT**: The sender device must transmit data in this exact structure:
```cpp
struct {
  uint8_t protocolVersion;  // Protocol version number
  float headTemp;           // Head temperature (not displayed on CYD)
  float oilTemp;            // Oil temperature in °F
  float oilPressure;        // Oil pressure in PSI
  uint32_t timestamp;       // Timestamp of reading
}
```

If your sender uses a different structure, you'll need to update the `SensorData` typedef in the receiver code to match.

## Notes
- GPS functionality remains unchanged
- ESP-NOW operates independently of GPS serial data
- Both GPS and sensor data update the display automatically
- The device will display GPS data even if sensor data is not available
