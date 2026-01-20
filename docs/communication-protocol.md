# Communication Protocol Specification
## Version 2

This document describes the ESP-NOW communication protocol used by the ESP32 Cylinder Head Temperature Monitor. Any receiver application must implement this protocol to correctly decode the data sent by the engine bay sender.

---

### Protocol Overview

*   **Transport**: ESP-NOW
*   **Packet Version**: 2
*   **Byte Order**: Little-endian (Standard ESP32/Arduino)
*   **Structure Packing**: Data is packed (no padding bytes)

### Data Packet Structure

The data packet must be defined as a packed struct to match the sender's memory layout.

**C/C++ Structure Definition:**

```cpp
#pragma pack(push, 1)
typedef struct {
  uint8_t version;         // Protocol version (Currently 2)
  uint32_t timestamp;      // Milliseconds since sender boot
  
  // Head Temperature Sensor
  float temperature;       // Temperature in Celsius
  float coldJunction;      // Cold junction temperature in Celsius
  uint8_t faultStatus;     // Head MAX31856 fault flags
  
  // Oil Temperature Sensor
  float oilTemperature;    // Temperature in Celsius
  float oilColdJunction;   // Cold junction temperature in Celsius
  uint8_t oilFaultStatus;  // Oil MAX31856 fault flags
  
  uint8_t sensorsStatus;   // Bitmask: Bit 0 = Head Present, Bit 1 = Oil Present
  uint16_t sequenceNumber; // Increments with every packet sent
  uint8_t batteryLevel;    // 0-100 (Future use, currently 0)
  uint8_t checksum;        // Exclusive-OR (XOR) checksum of all previous bytes
} TempDataPacket;
#pragma pack(pop)
```

### Field Descriptions

| Field | Type | Description |
| :--- | :--- | :--- |
| **version** | `uint8_t` | Must be **2**. If not 2, discard packet. |
| **timestamp** | `uint32_t` | Sender uptime in milliseconds. |
| **temperature** | `float` | **Head** thermocouple temp (°C). |
| **coldJunction** | `float` | **Head** amplifier internal temp (°C). |
| **faultStatus** | `uint8_t` | **Head** error flags (see Fault Codes below). |
| **oilTemperature** | `float` | **Oil** thermocouple temp (°C). |
| **oilColdJunction** | `float` | **Oil** amplifier internal temp (°C). |
| **oilFaultStatus** | `uint8_t` | **Oil** error flags. |
| **oilPressure** | `float` | **Oil Pressure** in PSI. |
| **sensorsStatus** | `uint8_t` | bit 0 (0x01)=Head, bit 1 (0x02)=Oil Temp, bit 2 (0x04)=Oil Press |
| **sequenceNumber** | `uint16_t` | Packet counter. Use to detect packet loss. |
| **checksum** | `uint8_t` | Integrity check. |

---

### Fault Codes

The `faultStatus` and `oilFaultStatus` bytes map directly to the MAX31856 fault register:

| Bit Mask | Name | Description |
| :--- | :--- | :--- |
| `0x01` | **OPEN** | Thermocouple open circuit (disconnected) |
| `0x02` | **OVUV** | Overvoltage / Undervoltage |
| `0x04` | **TCLOW** | Thermocouple Critical Low |
| `0x08` | **TCHIGH** | Thermocouple Critical High |
| `0x10` | **CJLOW** | Cold Junction Low |
| `0x20` | **CJHIGH** | Cold Junction High |
| `0x40` | **TCRANGE** | Thermocouple Out of Range |
| `0x80` | **CJRANGE** | Cold Junction Out of Range |

**Note**: `0x00` means **OK** (No Faults).

---

### Checksum Algorithm

The checksum is a simple XOR of all bytes in the packet *before* the checksum field.

**Validation Code (C++):**
```cpp
uint8_t calculateChecksum(const TempDataPacket *packet) {
  uint8_t checksum = 0;
  const uint8_t *data = (const uint8_t *)packet;
  // XOR all bytes except the last one (which is the checksum itself)
  for (size_t i = 0; i < sizeof(TempDataPacket) - 1; i++) {
    checksum ^= data[i];
  }
  return checksum;
}
```

### Receiver Implementation Checklist

1.  **Initialize ESP-NOW**: Set device to Station mode (`WIFI_STA`) and init ESP-NOW.
2.  **Register Callback**: Register an `OnDataRecv` callback.
3.  **Validate Length**: Ensure received data length equals `sizeof(TempDataPacket)`.
4.  **Validate Checksum**: Calculate XOR sum of the data; match with `packet.checksum`.
5.  **Validate Version**: Ensure `packet.version == 2`.
6.  **Decode Data**: Read temperatures and flags.
7.  **Check Sensor Status**: Use `sensorsStatus` to determine which temperatures are valid to display.
