#include <esp_now.h>
#include <WiFi.h>
#include <Preferences.h>
#include "fuel_config.h"
#include "fuel_data_packet.h"

// ============================================================================
// Global Variables
// ============================================================================

Preferences prefs;
FuelDataPacket fuel_packet;
uint16_t sequence_counter = 0;
float smoothed_resistance = 0.0;
uint32_t last_sample_time = 0;
uint32_t last_transmit_time = 0;
uint32_t last_display_update_time = 0;
bool smoothing_initialized = false;

// Calibration offsets (loaded from Preferences)
float empty_ohms_offset = 0.0;
float full_ohms_offset = 0.0;
int low_fuel_threshold = LOW_FUEL_THRESHOLD_PERCENT;

// MAC address of CYD display (receiver)
uint8_t cyd_mac[6] = CYD_MAC_ADDR;

// ============================================================================
// Function Declarations
// ============================================================================

void init_adc();
void init_espnow();
void init_preferences();
float read_fuel_resistance();
uint8_t resistance_to_percent(float resistance);
void update_fuel_packet();
void transmit_fuel_packet();
void on_espnow_sent(const uint8_t *mac_addr, esp_now_send_status_t status);
void process_serial_menu();

// ============================================================================
// Setup & Initialization
// ============================================================================

void setup() {
  // Serial for debugging and calibration menu
  Serial.begin(SERIAL_BAUD);
  delay(100);
  
  Serial.println("\n\n=== Fuel Sender Initialized ===");
  Serial.println("1972 VW Superbeetle Fuel Level Monitor");
  Serial.println("XIAO ESP32C6 + ESP-NOW");
  
  // Initialize hardware
  init_adc();
  init_preferences();
  init_espnow();
  
  Serial.println("Setup complete. Type 'menu' for calibration options.\n");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  uint32_t now = millis();
  
  // Handle serial input (calibration menu)
  if (Serial.available()) {
    process_serial_menu();
  }
  
  // Read ADC and smooth resistance value
  if (now - last_sample_time >= SAMPLE_INTERVAL_MS) {
    last_sample_time = now;
    
    float raw_resistance = read_fuel_resistance();
    
    // Initialize smoothing on first valid read
    if (!smoothing_initialized) {
      smoothed_resistance = raw_resistance;
      smoothing_initialized = true;
    } else {
      // Exponential averaging
      smoothed_resistance = (FUEL_SMOOTHING_ALPHA * raw_resistance) + 
                           ((1.0 - FUEL_SMOOTHING_ALPHA) * smoothed_resistance);
    }
  }
  
  // Transmit packet at 1 Hz
  if (now - last_transmit_time >= ESPNOW_TRANSMIT_INTERVAL_MS) {
    last_transmit_time = now;
    update_fuel_packet();
    transmit_fuel_packet();
  }
  
  // Update local OLED display (if enabled)
  #if ENABLE_OLED_DISPLAY
  if (now - last_display_update_time >= DISPLAY_UPDATE_INTERVAL_MS) {
    last_display_update_time = now;
    // TODO: Update OLED display with fuel_packet data
  }
  #endif
  
  delay(10);  // Small delay to prevent watchdog issues
}

// ============================================================================
// ADC Initialization
// ============================================================================

void init_adc() {
  // Configure ADC with full attenuation (0-3.3V range)
  analogReadResolution(12);           // 12-bit ADC (0-4095)
  analogSetAttenuation(ADC_ATTEN_DB_11);  // Full 3.3V range
  
  Serial.println("ADC initialized (GPIO3, 12-bit, 3.3V range)");
}

// ============================================================================
// Preferences (NV Storage) Initialization
// ============================================================================

void init_preferences() {
  prefs.begin(PREFS_NAMESPACE, false);  // false = RW mode
  
  // Load calibration offsets from flash
  empty_ohms_offset = prefs.getFloat(PREFS_EMPTY_OFFSET, 0.0);
  full_ohms_offset = prefs.getFloat(PREFS_FULL_OFFSET, 0.0);
  low_fuel_threshold = prefs.getInt(PREFS_LOW_FUEL_THRESHOLD, LOW_FUEL_THRESHOLD_PERCENT);
  
  Serial.print("Loaded calibration - Empty offset: ");
  Serial.print(empty_ohms_offset);
  Serial.print(" Ω, Full offset: ");
  Serial.print(full_ohms_offset);
  Serial.print(" Ω, Low fuel threshold: ");
  Serial.println(low_fuel_threshold + "%");
}

// ============================================================================
// ESP-NOW Initialization
// ============================================================================

void init_espnow() {
  // Set WiFi mode and channel
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  WiFi.disconnect();  // Disable WiFi scanning
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed!");
    return;
  }
  
  // Register send callback
  esp_now_register_send_cb(on_espnow_sent);
  
  // Add CYD display as peer
  esp_now_peer_info_t peer_info = {};
  memcpy(peer_info.peer_addr, cyd_mac, 6);
  peer_info.channel = ESPNOW_WIFI_CHANNEL;
  peer_info.encrypt = false;
  
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("ERROR: Failed to add CYD display peer!");
    return;
  }
  
  Serial.println("ESP-NOW initialized, CYD display peer added");
  Serial.print("  CYD MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", cyd_mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

// ============================================================================
// ADC Reading & Resistance Calculation
// ============================================================================

/**
 * Read fuel sender resistance from ADC
 * Converts raw ADC value to resistance using voltage divider formula
 * 
 * Voltage divider: VCC -- [R_series] -- ADC_input -- [Fuel_Sender] -- GND
 * 
 * V_adc = VCC * Rfuel / (Rseries + Rfuel)
 * Solving for Rfuel:
 * Rfuel = Rseries * Vadc / (VCC - Vadc)
 */
float read_fuel_resistance() {
  int adc_raw = analogRead(FUEL_ADC_PIN);
  
  // Convert ADC to voltage (0-3.3V)
  float voltage = (adc_raw / 4095.0) * VOLTAGE_DIVIDER_VCC;
  
  // Prevent division by zero
  if (voltage >= VOLTAGE_DIVIDER_VCC) {
    return FUEL_CLAMP_MAX_OHMS * 1.5;  // Return high value for fault detection
  }
  
  // Calculate resistance using voltage divider formula
  float resistance = VOLTAGE_DIVIDER_SERIES * voltage / (VOLTAGE_DIVIDER_VCC - voltage);
  
  // Clamp to valid range
  resistance = constrain(resistance, FUEL_CLAMP_MIN_OHMS, FUEL_CLAMP_MAX_OHMS);
  
  return resistance;
}

// ============================================================================
// Resistance to Percentage Conversion
// ============================================================================

/**
 * Convert resistance (ohms) to fuel percentage (0-100)
 * 
 * 1972 VW Superbeetle:
 *   73 Ω = Empty (0%)
 *   10 Ω = Full (100%)
 *   Range = 63 Ω
 * 
 * Formula (inverted because lower resistance = more fuel):
 *   fuel% = (73 - resistance) / 63 * 100
 * 
 * With calibration offsets applied:
 *   fuel% = (73 + empty_offset - resistance) / (63 + offset_range) * 100
 */
uint8_t resistance_to_percent(float resistance) {
  // Apply calibration offsets
  float adjusted_empty = FUEL_OHMS_EMPTY + empty_ohms_offset;
  float adjusted_full = FUEL_OHMS_FULL + full_ohms_offset;
  float adjusted_range = adjusted_empty - adjusted_full;
  
  // Prevent division by zero
  if (adjusted_range <= 0) {
    adjusted_range = FUEL_RESISTANCE_RANGE;  // Fallback to uncalibrated range
  }
  
  // Calculate percentage (inverted: 73Ω = 0%, 10Ω = 100%)
  float fuel_percent = ((adjusted_empty - resistance) / adjusted_range) * 100.0;
  
  // Clamp to 0-100%
  fuel_percent = constrain(fuel_percent, 0.0, 100.0);
  
  return (uint8_t)round(fuel_percent);
}

// ============================================================================
// Packet Update & Transmission
// ============================================================================

/**
 * Update fuel_packet with current sensor data and fault status
 */
void update_fuel_packet() {
  // Version and timestamp
  fuel_packet.version = 1;
  fuel_packet.timestamp = millis();
  
  // Raw resistance (clamped to valid range)
  float clamped_resistance = constrain(smoothed_resistance, FUEL_CLAMP_MIN_OHMS, FUEL_CLAMP_MAX_OHMS);
  fuel_packet.raw_resistance = (uint16_t)round(clamped_resistance * 100);  // Store as 0.01Ω units
  
  // Calculate fuel percentage
  fuel_packet.fuel_percent = resistance_to_percent(smoothed_resistance);
  
  // Determine fault status
  fuel_packet.fault_status = FUEL_FAULT_NONE;
  
  #if ENABLE_FAULT_DETECTION
  if (smoothed_resistance > FUEL_FAULT_OPEN_CIRCUIT_OHMS) {
    fuel_packet.fault_status |= FUEL_FAULT_OPEN_CIRCUIT;
  } else if (smoothed_resistance < FUEL_FAULT_SHORT_CIRCUIT_OHMS) {
    fuel_packet.fault_status |= FUEL_FAULT_SHORT_CIRCUIT;
  }
  
  if (fuel_packet.fuel_percent < low_fuel_threshold) {
    fuel_packet.fault_status |= FUEL_FAULT_LOW_FUEL;
  }
  #endif
  
  // Sequence number
  fuel_packet.sequence_number = sequence_counter++;
  
  // Calculate checksum
  const uint8_t* pkt_bytes = (const uint8_t*)&fuel_packet;
  fuel_packet.checksum = 0;
  for (size_t i = 0; i < offsetof(FuelDataPacket, checksum); i++) {
    fuel_packet.checksum ^= pkt_bytes[i];
  }
}

/**
 * Transmit fuel_packet via ESP-NOW with retry logic
 */
void transmit_fuel_packet() {
  uint8_t retry_count = 0;
  esp_err_t result = ESP_FAIL;
  
  while (retry_count < ESPNOW_RETRY_ATTEMPTS && result != ESP_OK) {
    result = esp_now_send(cyd_mac, (uint8_t *)&fuel_packet, sizeof(FuelDataPacket));
    
    if (result != ESP_OK) {
      retry_count++;
      if (retry_count < ESPNOW_RETRY_ATTEMPTS) {
        delay(ESPNOW_RETRY_DELAY_MS);
      }
    }
  }
  
  if (result != ESP_OK) {
    Serial.print("ERROR: ESP-NOW send failed (");
    Serial.print(result);
    Serial.println(")");
  }
}

/**
 * ESP-NOW send callback
 */
void on_espnow_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: Log transmission status
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("WARN: Packet transmission failed");
  }
}

// ============================================================================
// Serial Calibration Menu
// ============================================================================

void process_serial_menu() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  input.toLowerCase();
  
  if (input == "menu") {
    Serial.println("\n--- Fuel Sender Calibration Menu ---");
    Serial.println("  'status'   - Show current resistance & fuel %");
    Serial.println("  'cal'      - Enter calibration mode");
    Serial.println("  'reset'    - Reset calibration to defaults");
    Serial.println("  'help'     - Show this menu");
    Serial.println();
    
  } else if (input == "status") {
    Serial.print("Current: ");
    Serial.print(smoothed_resistance, 1);
    Serial.print(" Ω -> ");
    Serial.print(fuel_packet.fuel_percent);
    Serial.print("% | Faults: 0x");
    Serial.print(fuel_packet.fault_status, HEX);
    Serial.print(" | Seq: ");
    Serial.println(fuel_packet.sequence_number);
    
  } else if (input == "cal") {
    calibration_menu();
    
  } else if (input == "reset") {
    prefs.remove(PREFS_EMPTY_OFFSET);
    prefs.remove(PREFS_FULL_OFFSET);
    prefs.remove(PREFS_LOW_FUEL_THRESHOLD);
    empty_ohms_offset = 0.0;
    full_ohms_offset = 0.0;
    low_fuel_threshold = LOW_FUEL_THRESHOLD_PERCENT;
    Serial.println("Calibration reset to defaults");
    
  } else if (input == "help" || input == "menu") {
    // Already handled above
  }
}

/**
 * Interactive calibration menu
 */
void calibration_menu() {
  Serial.println("\n--- Calibration Mode ---");
  Serial.println("Measure fuel sender resistance at known levels:");
  Serial.println();
  
  Serial.println("1. Empty tank test:");
  Serial.println("   Connect fuel sender when tank is EMPTY (73 Ω nominal)");
  Serial.println("   Resistance reading: ");
  // TODO: Implement interactive calibration
  
  Serial.println("\nNote: Use 'status' to monitor live resistance readings");
  Serial.println("Type 'exit' to return to main menu");
}

