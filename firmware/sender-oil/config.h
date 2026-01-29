#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// SPI PINS FOR XIAO ESP32C6 -> MAX31856
// ============================================================================
// ============================================================================
// ============================================================================
// SPI PINS FOR XIAO ESP32C6 -> MAX31856 (Full SPI with MOSI)
// ============================================================================
#define MAX31856_CS_PIN 21 // D3 - Chip Select (Head Temp) (GPIO21)
#define SPI_SCK_PIN 19     // D8 - SPI Clock (GPIO19)
#define SPI_MISO_PIN 20    // D9 - SPI MISO (GPIO20)
#define SPI_MOSI_PIN 18    // D10 - SPI MOSI (GPIO18 - REQUIRED for MAX31856)

// ============================================================================
// I2C PINS FOR OLED DISPLAY (XIAO ESP32C6)
// ============================================================================
#define OLED_SDA_PIN 22   // D4 - I2C Data (GPIO22)
#define OLED_SCL_PIN 23   // D5 - I2C Clock (GPIO23)
#define OLED_ADDR 0x3C    // I2C address for SSD1306 OLED (change if different)
#define ADS1115_ADDR 0x48 // I2C address for ADS1115 ADC (Default: 0x48)

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
#define SCREEN_WIDTH 128 // OLED display width in pixels
#define SCREEN_HEIGHT 64 // OLED display height in pixels

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================
#define SAMPLE_INTERVAL_MS                                                     \
  500 // Sample temperature every 500ms (2 Hz) for stability
#define TRANSMIT_INTERVAL_MS                                                   \
  1000 // Transmit every 1 second (1 Hz) to save bandwidth
#define DISPLAY_UPDATE_INTERVAL_MS 100 // Update display every 100ms

// ============================================================================
// ESP-NOW CONFIGURATION
// ============================================================================
#define ESPNOW_CHANNEL 1  // Wi-Fi channel (1-13)
#define MAX_RETRY_COUNT 3 // Maximum retries for failed transmissions

// ============================================================================
// RECEIVER MAC ADDRESS
// ============================================================================
// IMPORTANT: You must update this with your receiver's MAC address
// To get the receiver MAC address:
// 1. Flash and run the receiver code first
// 2. Open Serial Monitor on the receiver
// 3. Copy the MAC address displayed on boot
// 4. Replace the values below
// Example: If MAC is AA:BB:CC:DD:EE:FF, use {0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
// 0xFF}
extern uint8_t receiverMAC[6]; // Defined in sender_arduino.ino

// ============================================================================
// TEMPERATURE THRESHOLDS (Optional - for future use)
// ============================================================================
#define TEMP_WARNING_THRESHOLD 150.0f  // Celsius - warn when exceeded
#define TEMP_CRITICAL_THRESHOLD 200.0f // Celsius - critical alert

// ============================================================================
// STATUS LED (Optional)
// ============================================================================
#define STATUS_LED_PIN -1 // Set to GPIO pin if you want LED status indicator

// ============================================================================
// PRESSURE SENSOR CONFIGURATION (0-100 PSI)
// ============================================================================
// Sensor specs: 0.5V = 0 PSI, 4.5V = 100 PSI (Typical 5V sensor)
// VOLTAGE DIVIDER IMPLEMENTED:
// - R1 (Series): 2.2k Ohm (between Sensor and ADS)
// - R2 (Ground): 4.7k Ohm (between ADS signal and Ground)
// - Divider Ratio: 4.7k / (2.2k + 4.7k) = ~0.68116
// - New Range: 0.5V * 0.68 = 0.34V | 4.5V * 0.68 = 3.07V
// We are using ADS1115 GAIN_TWOTHIRDS (Range +/- 6.144V)
#define SENSOR_MIN_VOLTAGE 0.34f
#define SENSOR_MAX_VOLTAGE 3.07f
#define SENSOR_MAX_PSI 100.0f

#endif // CONFIG_H
