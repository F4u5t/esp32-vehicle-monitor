#include <Arduino.h>
#include <Preferences.h>
#include "fuel_config.h"
#include "fuel_data_packet.h"

// ============================================================================
// Fuel Sensor Calibration Implementation
// Provides interactive serial menu for field calibration
// ============================================================================

extern Preferences prefs;
extern float smoothed_resistance;
extern float empty_ohms_offset;
extern float full_ohms_offset;
extern int low_fuel_threshold;
extern FuelDataPacket fuel_packet;

static const char* PREFS_NAMESPACE = "fuel_sender";

// ============================================================================
// Calibration Data Structure
// ============================================================================

typedef struct {
  float empty_measured;     // Measured resistance at empty
  float full_measured;      // Measured resistance at full
  int samples_taken;        // Number of readings averaged
  bool is_valid;            // Calibration is valid
} CalibrationData;

CalibrationData cal_data = {0.0, 0.0, 0, false};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Get averaged resistance reading (for calibration)
 * Takes multiple samples and returns median for stability
 */
float get_averaged_resistance(int num_samples = 10) {
  float readings[num_samples];
  
  Serial.print("Sampling ");
  Serial.print(num_samples);
  Serial.print(" readings...");
  
  for (int i = 0; i < num_samples; i++) {
    // In real implementation, read_fuel_resistance() would be available here
    // For now, use smooothed_resistance as proxy
    readings[i] = smoothed_resistance;
    delay(100);
    Serial.print(".");
  }
  Serial.println(" done.");
  
  // Sort for median calculation
  for (int i = 0; i < num_samples - 1; i++) {
    for (int j = 0; j < num_samples - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }
  
  // Return median (middle value or average of two middle for even count)
  if (num_samples % 2 == 0) {
    return (readings[num_samples / 2 - 1] + readings[num_samples / 2]) / 2.0;
  } else {
    return readings[num_samples / 2];
  }
}

// ============================================================================
// Calibration Menu Functions
// ============================================================================

/**
 * Display current calibration status
 */
void show_calibration_status() {
  Serial.println("\n=== Current Calibration Status ===");
  Serial.print("Empty offset: ");
  Serial.print(empty_ohms_offset, 2);
  Serial.println(" Ω");
  
  Serial.print("Full offset: ");
  Serial.print(full_ohms_offset, 2);
  Serial.println(" Ω");
  
  Serial.print("Low fuel threshold: ");
  Serial.print(low_fuel_threshold);
  Serial.println("%");
  
  Serial.print("\nCurrent resistance: ");
  Serial.print(smoothed_resistance, 1);
  Serial.println(" Ω");
  
  Serial.print("Current fuel level: ");
  Serial.print(fuel_packet.fuel_percent);
  Serial.println("%");
  
  Serial.print("Fault flags: 0x");
  Serial.println(fuel_packet.fault_status, HEX);
}

/**
 * Two-point calibration (empty + full)
 * Most accurate method
 */
void calibrate_two_point() {
  Serial.println("\n=== Two-Point Calibration ===");
  Serial.println("This method uses empty and full tank measurements for best accuracy.");
  Serial.println();
  
  // Step 1: Empty tank calibration
  Serial.println("STEP 1: Empty Tank Calibration");
  Serial.println("Disconnect fuel sender if needed, or position at tank empty.");
  Serial.println("Expected resistance: ~73 Ω");
  Serial.println("Press ENTER when ready...");
  while (Serial.available() == 0) delay(10);
  Serial.readStringUntil('\n');
  
  float empty_reading = get_averaged_resistance(10);
  Serial.print("Empty tank resistance: ");
  Serial.print(empty_reading, 2);
  Serial.println(" Ω");
  
  // Step 2: Full tank calibration
  Serial.println("\nSTEP 2: Full Tank Calibration");
  Serial.println("Connect fuel sender at full tank position.");
  Serial.println("Expected resistance: ~10 Ω");
  Serial.println("Press ENTER when ready...");
  while (Serial.available() == 0) delay(10);
  Serial.readStringUntil('\n');
  
  float full_reading = get_averaged_resistance(10);
  Serial.print("Full tank resistance: ");
  Serial.print(full_reading, 2);
  Serial.println(" Ω");
  
  // Step 3: Calculate offsets
  // empty_offset brings our measured 73Ω to match the actual VW spec
  // full_offset brings our measured 10Ω to match the actualy VW spec
  empty_ohms_offset = FUEL_OHMS_EMPTY - empty_reading;
  full_ohms_offset = FUEL_OHMS_FULL - full_reading;
  
  Serial.println("\n=== Calibration Complete ===");
  Serial.print("Empty offset: ");
  Serial.print(empty_ohms_offset, 2);
  Serial.println(" Ω");
  
  Serial.print("Full offset: ");
  Serial.print(full_ohms_offset, 2);
  Serial.println(" Ω");
  
  // Save to preferences
  save_calibration();
  
  Serial.println("Calibration saved to flash.");
}

/**
 * Single-point calibration (use known reference only)
 */
void calibrate_single_point() {
  Serial.println("\n=== Single-Point Calibration ===");
  Serial.println("This method uses one known reference point (empty or full).");
  Serial.println();
  
  Serial.println("Which reference do you have?");
  Serial.println("  1) Empty tank (73 Ω nominal)");
  Serial.println("  2) Full tank (10 Ω nominal)");
  Serial.println("  3) Custom resistance");
  Serial.println("Select (1-3): ");
  
  while (Serial.available() == 0) delay(10);
  char choice = Serial.read();
  Serial.readStringUntil('\n');
  
  float measured = get_averaged_resistance(10);
  Serial.print("Measured resistance: ");
  Serial.print(measured, 2);
  Serial.println(" Ω");
  
  if (choice == '1') {
    // Empty reference
    empty_ohms_offset = FUEL_OHMS_EMPTY - measured;
    Serial.print("Empty offset set to: ");
    Serial.println(empty_ohms_offset, 2);
    
  } else if (choice == '2') {
    // Full reference
    full_ohms_offset = FUEL_OHMS_FULL - measured;
    Serial.print("Full offset set to: ");
    Serial.println(full_ohms_offset, 2);
    
  } else if (choice == '3') {
    // Custom reference
    Serial.println("Enter reference resistance in ohms (e.g., 73.5): ");
    while (Serial.available() == 0) delay(10);
    String ref_str = Serial.readStringUntil('\n');
    float reference = ref_str.toFloat();
    
    float offset = reference - measured;
    Serial.print("Offset for this point: ");
    Serial.println(offset, 2);
  }
  
  save_calibration();
  Serial.println("Calibration saved.");
}

/**
 * Manual offset adjustment
 * Fine-tune existing calibration
 */
void adjust_offsets_manual() {
  Serial.println("\n=== Manual Offset Adjustment ===");
  Serial.println("Fine-tune calibration with precise offset values.");
  Serial.println();
  
  Serial.print("Current empty offset: ");
  Serial.println(empty_ohms_offset, 3);
  Serial.println("Enter new empty offset (in Ω, or press ENTER to skip): ");
  
  while (Serial.available() == 0) delay(10);
  String input = Serial.readStringUntil('\n');
  if (input.length() > 0) {
    empty_ohms_offset = input.toFloat();
    Serial.print("Empty offset updated to: ");
    Serial.println(empty_ohms_offset, 3);
  }
  
  Serial.print("\nCurrent full offset: ");
  Serial.println(full_ohms_offset, 3);
  Serial.println("Enter new full offset (in Ω, or press ENTER to skip): ");
  
  while (Serial.available() == 0) delay(10);
  input = Serial.readStringUntil('\n');
  if (input.length() > 0) {
    full_ohms_offset = input.toFloat();
    Serial.print("Full offset updated to: ");
    Serial.println(full_ohms_offset, 3);
  }
  
  save_calibration();
  Serial.println("Offsets saved.");
}

/**
 * Configure low fuel warning threshold
 */
void configure_low_fuel_threshold() {
  Serial.println("\n=== Low Fuel Warning Threshold ===");
  Serial.print("Current threshold: ");
  Serial.print(low_fuel_threshold);
  Serial.println("%");
  
  Serial.println("Enter new threshold percentage (5-25%), or press ENTER to skip: ");
  
  while (Serial.available() == 0) delay(10);
  String input = Serial.readStringUntil('\n');
  
  if (input.length() > 0) {
    int new_threshold = input.toInt();
    if (new_threshold >= 5 && new_threshold <= 25) {
      low_fuel_threshold = new_threshold;
      save_calibration();
      Serial.print("Low fuel threshold updated to: ");
      Serial.print(low_fuel_threshold);
      Serial.println("%");
    } else {
      Serial.println("ERROR: Threshold must be between 5-25%");
    }
  }
}

/**
 * Save all calibration data to Preferences (flash)
 */
void save_calibration() {
  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE, false);  // false = RW mode
  
  prefs.putFloat(PREFS_EMPTY_OFFSET, empty_ohms_offset);
  prefs.putFloat(PREFS_FULL_OFFSET, full_ohms_offset);
  prefs.putInt(PREFS_LOW_FUEL_THRESHOLD, low_fuel_threshold);
  
  prefs.end();
  Serial.println("✓ Calibration saved to flash memory");
}

/**
 * Main calibration menu loop
 */
void calibration_menu() {
  bool in_menu = true;
  
  while (in_menu) {
    Serial.println("\n=== Fuel Sender Calibration Menu ===");
    Serial.println("  1) Show calibration status");
    Serial.println("  2) Two-point calibration (RECOMMENDED)");
    Serial.println("  3) Single-point calibration");
    Serial.println("  4) Manual offset adjustment");
    Serial.println("  5) Configure low fuel threshold");
    Serial.println("  6) Reset calibration to defaults");
    Serial.println("  7) Exit calibration");
    Serial.println();
    Serial.print("Select option (1-7): ");
    
    while (Serial.available() == 0) delay(10);
    char choice = Serial.read();
    Serial.readStringUntil('\n');
    
    switch (choice) {
      case '1':
        show_calibration_status();
        break;
        
      case '2':
        calibrate_two_point();
        break;
        
      case '3':
        calibrate_single_point();
        break;
        
      case '4':
        adjust_offsets_manual();
        break;
        
      case '5':
        configure_low_fuel_threshold();
        break;
        
      case '6':
        Serial.println("\nResetting calibration to defaults...");
        empty_ohms_offset = 0.0;
        full_ohms_offset = 0.0;
        low_fuel_threshold = LOW_FUEL_THRESHOLD_PERCENT;
        save_calibration();
        Serial.println("Calibration reset.");
        break;
        
      case '7':
        in_menu = false;
        Serial.println("Exiting calibration menu...\n");
        break;
        
      default:
        Serial.println("Invalid selection. Please try again.");
        break;
    }
  }
}

