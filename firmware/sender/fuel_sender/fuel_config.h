#ifndef FUEL_CONFIG_H
#define FUEL_CONFIG_H

// ============================================================================
// HARDWARE CONFIGURATION - Seeed Studio XIAO ESP32C6 Fuel Sender
// ============================================================================

// ADC Configuration
#define FUEL_ADC_PIN GPIO_NUM_3          // ADC input for fuel sender resistance
#define ADC_ATTENUATION ADC_ATTEN_DB_11  // Full range 0-3.3V
#define ADC_CHANNELS ADC_CHANNEL_3       // GPIO3 = ADC1_CH3

// I2C Configuration (if using external ADS1115 - alternative option)
#define I2C_SDA GPIO_NUM_1               // XIAO C6 SDA
#define I2C_SCL GPIO_NUM_2               // XIAO C6 SCL
#define I2C_FREQ 400000                  // 400 kHz
#define ADS1115_ADDR 0x48                // I2C address (ADR->GND)

// Fuel Sender Specifications
// 1972 VW Superbeetle: 73 ohms (empty) -> 10 ohms (full)
#define FUEL_OHMS_EMPTY 73
#define FUEL_OHMS_FULL 10
#define FUEL_RESISTANCE_RANGE (FUEL_OHMS_EMPTY - FUEL_OHMS_FULL)  // 63 ohms

// Voltage Divider Configuration
// Using fixed resistor + fuel sender as voltage divider
// Rseries = 100 ohms (chosen to get ~1.65V at midpoint on 3.3V rail)
#define VOLTAGE_DIVIDER_SERIES 100.0     // Series resistor in ohms
#define VOLTAGE_DIVIDER_VCC 3.3          // Supply voltage

// Noise Filtering
#define ADC_FILTER_CAPACITOR_UF 0.1      // 100nF capacitor on ADC input
#define MAJORITY_VOTE_THRESHOLD 3        // Use 3-reading consensus for fault detection

// Timing Configuration
#define SAMPLE_INTERVAL_MS 500           // Read ADC at 2 Hz
#define TRANSMIT_INTERVAL_MS 1000        // Send ESP-NOW packet at 1 Hz (async from sampling)
#define DISPLAY_UPDATE_INTERVAL_MS 100   // Update local OLED at 10 Hz

// Smoothing (Exponential Averaging)
#define FUEL_SMOOTHING_ALPHA 0.2         // Same as oil temperature sensor
#define MIN_VALID_SAMPLES 2              // Minimum readings before using smoothed value

// Fault Detection & Safety
#define FUEL_FAULT_OPEN_CIRCUIT_OHMS 100.0   // Resistance > this = open circuit
#define FUEL_FAULT_SHORT_CIRCUIT_OHMS 5.0    // Resistance < this = short circuit
#define FUEL_CLAMP_MIN_OHMS 5.0              // Lower bound for clamping
#define FUEL_CLAMP_MAX_OHMS 100.0            // Upper bound for clamping

// Low Fuel Warning
#define LOW_FUEL_THRESHOLD_PERCENT 15    // Alert when fuel drops below this %

// Serial Configuration
#define SERIAL_BAUD 115200
#define SERIAL_RX GPIO_NUM_20            // XIAO C6 RX
#define SERIAL_TX GPIO_NUM_21            // XIAO C6 TX

// OLED Display Configuration (optional local display)
#define OLED_ADDR 0x3C                   // SSD1306 I2C address
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// ESP-NOW Configuration
#define ESPNOW_WIFI_CHANNEL 1            // Same channel as oil sender
#define ESPNOW_TRANSMIT_INTERVAL_MS 1000 // 1 Hz transmission
#define ESPNOW_RETRY_ATTEMPTS 3
#define ESPNOW_RETRY_DELAY_MS 50

// CYD Display MAC Address (receiver)
// This is the MAC of the Cheap Yellow Display that receives fuel packets
#define CYD_MAC_ADDR {0x08, 0xD1, 0xF9, 0x2A, 0x0B, 0xBC}

// Preferences (Non-volatile storage)
#define PREFS_NAMESPACE "fuel_sender"
#define PREFS_EMPTY_OFFSET "fuel_empty_offset"      // Calibration offset for empty
#define PREFS_FULL_OFFSET "fuel_full_offset"        // Calibration offset for full
#define PREFS_LOW_FUEL_THRESHOLD "low_fuel_thresh"  // Configurable low fuel alert

// ADC Raw Value Range (for internal scaling)
#define ADC_MAX_RAW_VALUE 4095           // 12-bit ADC
#define ADC_REF_VOLTAGE 3.3              // Millivolts per conversion step
#define ADC_CONVERSION_STEP (ADC_REF_VOLTAGE / ADC_MAX_RAW_VALUE)

// Feature Flags
#define ENABLE_OLED_DISPLAY 1            // Set to 1 to enable local OLED, 0 to disable
#define ENABLE_SERIAL_MENU 1             // Set to 1 to enable calibration menu
#define ENABLE_FAULT_DETECTION 1         // Set to 1 to detect open/short circuits

#endif // FUEL_CONFIG_H
