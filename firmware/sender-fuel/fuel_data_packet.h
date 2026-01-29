#ifndef FUEL_DATA_PACKET_H
#define FUEL_DATA_PACKET_H

#include <stdint.h>

// ============================================================================
// Fuel Data Packet Definition (Protocol v1)
// Transmitted via ESP-NOW from fuel sender to CYD display
// Max payload: 250 bytes (conservative for ESP-NOW compatibility)
// ============================================================================

typedef struct __attribute__((packed)) {
  // === Packet Header ===
  uint8_t version;              // Protocol version = 1 (fuel sender)
  uint32_t timestamp;           // millis() when packet was created
  
  // === Fuel Sender Data ===
  uint16_t raw_resistance;      // Raw resistance in ohms (0-200, uint16 for precision)
  uint8_t fuel_percent;         // Calculated fuel level 0-100%
  
  // === Status & Diagnostics ===
  uint8_t fault_status;         // Bit flags for fault conditions:
                                //   0x01 = Open circuit (resistance too high)
                                //   0x02 = Short circuit (resistance too low)
                                //   0x04 = Reserved for future sensor faults
                                //   0x08 = Low fuel warning (< threshold)
                                //   0xF0 = Reserved
  
  // === Sequence & Integrity ===
  uint16_t sequence_number;     // Packet counter (0-65535, rolls over)
  uint8_t checksum;             // XOR checksum of all previous bytes
  
  // === Calibration Metadata (optional) ===
  uint8_t reserved;             // For future expansion (padding)
  
} FuelDataPacket;

// Packet size validation (must be < 250 bytes for ESP-NOW)
_Static_assert(sizeof(FuelDataPacket) < 250, "FuelDataPacket exceeds ESP-NOW max payload");

// ============================================================================
// Fault Status Bit Definitions
// ============================================================================

#define FUEL_FAULT_NONE 0x00
#define FUEL_FAULT_OPEN_CIRCUIT 0x01    // Sender disconnected or open wire
#define FUEL_FAULT_SHORT_CIRCUIT 0x02   // Sender shorted or very low resistance
#define FUEL_FAULT_SENSOR_ERROR 0x04    // Reserved for sensor-level errors
#define FUEL_FAULT_LOW_FUEL 0x08        // Level below warning threshold
#define FUEL_FAULT_RESERVED_MASK 0xF0   // Reserved for future use

// ============================================================================
// Packet Helper Functions (inline)
// ============================================================================

/**
 * Calculate XOR checksum for a FuelDataPacket
 * Checksums bytes 0 through 'fault_status' field (excludes sequence_number, checksum, reserved)
 */
static inline uint8_t fuel_packet_checksum(const uint8_t* packet_bytes, size_t num_bytes) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < num_bytes; i++) {
    checksum ^= packet_bytes[i];
  }
  return checksum;
}

/**
 * Validate a received FuelDataPacket's checksum and version
 * Returns 1 if valid, 0 if invalid
 */
static inline uint8_t fuel_packet_is_valid(const FuelDataPacket* pkt) {
  // Check version
  if (pkt->version != 1) {
    return 0;
  }
  
  // Verify checksum (calculate on all bytes except checksum field itself)
  const uint8_t* pkt_bytes = (const uint8_t*)pkt;
  uint8_t calculated_checksum = fuel_packet_checksum(pkt_bytes, offsetof(FuelDataPacket, checksum));
  
  if (calculated_checksum != pkt->checksum) {
    return 0;
  }
  
  return 1;
}

#endif // FUEL_DATA_PACKET_H
