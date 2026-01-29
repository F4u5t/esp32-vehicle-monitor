#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <stdint.h>

// ============================================================================
// PROTOCOL CONFIGURATION
// ============================================================================
// Protocol version for future compatibility
#define PROTOCOL_VERSION 3

// Maximum ESP-NOW payload: 250 bytes (v1.0) or 1470 bytes (v2.0+)
// Using conservative size for v1.0 compatibility
#define MAX_ESPNOW_DATA_LEN 250

// ============================================================================
// DATA PACKET STRUCTURE
// ============================================================================
// Data packet structure for temperature transmission
// CRITICAL: This structure MUST be IDENTICAL on both sender and receiver!
typedef struct __attribute__((packed)) {
  uint8_t version;     // Protocol version (should be PROTOCOL_VERSION)
  uint32_t timestamp;  // Milliseconds since boot (from millis())
  float temperature;   // Head Temperature in Celsius
  float coldJunction;  // Head Cold junction temperature in Celsius
  uint8_t faultStatus; // Head MAX31856 fault register

  float oilTemperature;   // Oil Temperature in Celsius
  float oilColdJunction;  // Oil Cold junction temperature in Celsius
  uint8_t oilFaultStatus; // Oil MAX31856 fault register

  float oilPressure;       // Oil Pressure in PSI
  uint8_t sensorsStatus;   // Bitmask: Bit 0=Head, 1=Oil Temp, 2=Oil Press
  uint16_t sequenceNumber; // Packet sequence number (increments each send)
  uint8_t batteryLevel;    // Battery percentage 0-100 (future use, currently 0)
  uint8_t checksum;        // Simple XOR checksum for data integrity validation
} TempDataPacket;

// ============================================================================
// CHECKSUM FUNCTIONS
// ============================================================================

// Calculate simple XOR checksum for packet integrity
inline uint8_t calculateChecksum(const TempDataPacket *packet) {
  uint8_t checksum = 0;
  const uint8_t *data = (const uint8_t *)packet;
  // XOR all bytes except the checksum field itself (last byte)
  for (size_t i = 0; i < sizeof(TempDataPacket) - 1; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Validate packet checksum
inline bool validatePacket(const TempDataPacket *packet) {
  return (packet->checksum == calculateChecksum(packet));
}

// ============================================================================
// MAX31855 FAULT BIT DEFINITIONS
// ============================================================================
// Fault bit definitions from MAX31855 datasheet
#define FAULT_OPEN 0x01      // Thermocouple open circuit
#define FAULT_SHORT_GND 0x02 // Thermocouple short to GND
#define FAULT_SHORT_VCC 0x04 // Thermocouple short to VCC
#define FAULT_GENERIC 0x07   // Generic/Multiple faults behavior

// ============================================================================
// HELPER FUNCTIONS FOR FAULT INTERPRETATION
// ============================================================================

// Check if any fault is present
inline bool hasFault(uint8_t faultStatus) { return (faultStatus != 0); }

// Check for open circuit (most common fault - disconnected)
inline bool isOpenCircuit(uint8_t faultStatus) {
  return (faultStatus & FAULT_OPEN);
}

// Get human-readable fault description (for debugging)
inline const char *getFaultDescription(uint8_t faultStatus) {
  if (faultStatus == 0)
    return "OK";
  if (faultStatus & FAULT_OPEN)
    return "OPEN CIRCUIT";
  if (faultStatus & FAULT_SHORT_GND)
    return "SHORT TO GND";
  if (faultStatus & FAULT_SHORT_VCC)
    return "SHORT TO VCC";
  if (faultStatus == 0xFF)
    return "READ ERROR";
  return "UNKNOWN FAULT";
}

#endif // DATA_PACKET_H
