/*
 * ESP32C6 Cylinder Head Temperature Monitor - SENDER
 *
 * Hardware:
 * - Seeed Studio XIAO ESP32C6
 * - Adafruit MAX31856 Thermocouple Amplifier
 * - K-type Thermocouple
 * - SSD1306 OLED Display (128x64, I2C) for local viewing
 *
 * Functionality:
 * - Reads temperature from MAX31856 via SPI
 * - Displays temperature locally on OLED (for engine bay work)
 * - Transmits temperature data via ESP-NOW to dash-mounted receiver
 * - Monitors for thermocouple faults
 * - Implements retry logic for failed transmissions
 */

#include "SSD1306Wire.h"
#include "config.h"
#include "console_menu.h"
#include "data_packet.h"
#include "settings.h"
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MAX31856.h>
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// ============================================================================
// GLOBAL OBJECTS AND VARIABLES
// ============================================================================
const float ALPHA =
    0.2f; // Smoothing factor (0.0 - 1.0). Lower = smoother but slower.

// Using Hardware SPI for MAX31856
#define SPI_MISO_PIN 20 // D9 - SPI MISO (GPIO20)
#define SPI_MOSI_PIN 18 // D10 - SPI MOSI (GPIO18 - REQUIRED for MAX31856)
// SCK=D8(19), MISO=D9(20), MOSI=D10(18), CS=D3(21)
// SCK=D8(19), MISO=D9(20), MOSI=D10(18), CS=D3(21)
Adafruit_MAX31856 max_oil(MAX31856_CS_PIN); // Oil Temperature

Adafruit_ADS1115 ads;
SSD1306Wire display(OLED_ADDR, OLED_SDA_PIN, OLED_SCL_PIN);

// Receiver MAC Address (from config.h)
// Update this with your RECEIVER's MAC address
uint8_t receiverMAC[] = {0x98, 0xA3, 0x16, 0x8E, 0x6A, 0xE4};

// Packet tracking
uint16_t sequenceNumber = 0;
unsigned long lastSampleTime = 0;
unsigned long lastTransmitTime = 0;
unsigned long lastDisplayUpdate = 0;

// Current readings (for display updates)
float currentOilTemperature = 0.0f;
float currentOilColdJunction = 0.0f;
uint8_t currentOilFaultStatus = 0;

float currentOilPressure = 0.0f;

bool dataValid = false;

// Sensor Detection
bool oilTempSensorFound = false;
bool pressureSensorFound = false;

// ESP-NOW send status tracking
bool sendSuccess = false;
int sendAttempts = 0;

// ============================================================================
// ESP-NOW CALLBACK: Called when data is sent
// ============================================================================
void onDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    sendSuccess = true;
    if (!isConsoleActive())
      Serial.println("✓ Delivery Success");
  } else {
    sendSuccess = false;
    if (!isConsoleActive())
      Serial.println("✗ Delivery Fail");
  }
}

// ============================================================================
// INITIALIZE ESP-NOW
// ============================================================================
bool initESPNow() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Print MAC address for debugging
  Serial.print("Sender MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("✗ ESP-NOW init failed");
    return false;
  }
  Serial.println("✓ ESP-NOW initialized");

  // Register send callback
  esp_now_register_send_cb(onDataSent);

  // Register peer (receiver)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false; // No encryption for simplicity

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("✗ Failed to add peer");
    return false;
  }
  Serial.println("✓ Peer added successfully");

  return true;
}

// ============================================================================
// UPDATE DISPLAY
// ============================================================================
void updateDisplay() {
  display.clear();

  int yOil = 0;
  int yPress = 21;

  // Oil Temp
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, yOil + 3, "Oil");

  display.setFont(ArialMT_Plain_16);
  if (hasFault(currentOilFaultStatus)) {
    display.drawString(40, yOil, "ERR: " + String(currentOilFaultStatus));
  } else {
    float tempF = (currentOilTemperature * 9.0 / 5.0) + 32.0;
    display.drawString(40, yOil, String(tempF, 1) + " F");
  }

  // Oil Pressure
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, yPress + 3, "Oil P");

  display.setFont(ArialMT_Plain_16);
  display.drawString(40, yPress, String(currentOilPressure, 1) + " PSI");

  // TX Status Indicator (Bottom Right)
  // Filled circle = Success, Empty circle = Fail/Retry
  if (sendSuccess) {
    display.fillCircle(124, 60, 3);
  } else {
    display.drawCircle(124, 60, 3);
  }

  display.display();
}

// ============================================================================
// SEND TEMPERATURE DATA VIA ESP-NOW
// ============================================================================
bool sendTemperatureData(float oilTemp, float oilCJ, uint8_t oilFault) {
  // Build data packet
  TempDataPacket packet;
  packet.version = PROTOCOL_VERSION;
  packet.timestamp = millis();
  packet.temperature = oilTemp;
  packet.coldJunction = oilCJ;
  packet.faultStatus = oilFault;

  // Head Temp Removed (now using Oil Temp)
  packet.oilTemperature = 0;
  packet.oilColdJunction = 0;
  packet.oilFaultStatus = 0;

  packet.oilPressure = currentOilPressure;

  packet.sensorsStatus =
      (oilTempSensorFound ? 0x01 : 0) | (pressureSensorFound ? 0x04 : 0);

  packet.sequenceNumber = sequenceNumber++;
  packet.batteryLevel = 0; // Future use
  packet.checksum = calculateChecksum(&packet);

  // Attempt to send with retries
  sendSuccess = false;
  for (sendAttempts = 0; sendAttempts < MAX_RETRY_COUNT; sendAttempts++) {
    esp_err_t result =
        esp_now_send(receiverMAC, (uint8_t *)&packet, sizeof(packet));

    if (result == ESP_OK) {
      // Wait briefly for callback
      delay(10);
      if (sendSuccess) {
        return true;
      }
    } else {
      if (!isConsoleActive())
        Serial.printf("✗ Send error (attempt %d/%d)\n", sendAttempts + 1,
                      MAX_RETRY_COUNT);
    }

    delay(50); // Brief delay before retry
  }

  if (!isConsoleActive())
    Serial.println("✗ All send attempts failed");
  return false;
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(2000); // Wait for Serial to be ready

  Serial.println("\n\n========================================");
  Serial.println("ESP32C6 Temperature Sender (Engine Bay)");
  Serial.println("========================================\n");

  // Load Settings
  SystemSettings.begin();

  // Initialize I2C and OLED display
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  Serial.println("I2C initialized on SDA=" + String(OLED_SDA_PIN) +
                 ", SCL=" + String(OLED_SCL_PIN));

  // Scan for I2C devices
  Serial.println("Scanning I2C bus...");
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("✗ No I2C devices found!");
    Serial.println("  Check OLED wiring:");
    Serial.println("    SDA -> D4 (GPIO6)");
    Serial.println("    SCL -> D5 (GPIO7)");
    Serial.println("    VCC -> 3.3V");
    Serial.println("    GND -> GND");
  } else {
    Serial.printf("✓ Found %d I2C device(s)\n", nDevices);
  }

  // Try to initialize display
  if (!display.init()) {
    Serial.println("✗ OLED display init failed!");
    Serial.println("  Try address 0x3D if 0x3C doesn't work");
  } else {
    display.flipScreenVertically(); // Adjust if needed based on mounting
    display.setContrast(255);       // Maximum brightness
    Serial.println("✓ OLED display initialized");
  }

  // Show splash screen
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 10, "Engine Bay");
  display.drawString(64, 30, "Temp Monitor");
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 50, "Initializing...");
  display.display();
  delay(2000);

  // Initialize MAX31856 (Oil Temp)
  // Init Hardware SPI first
  SPI.begin(19, SPI_MISO_PIN, SPI_MOSI_PIN, 21); // SCK, MISO, MOSI, CS

  if (!max_oil.begin()) {
    Serial.println("✗ MAX31856 (Oil Temp) init failed!");
    // (Wiring check omitted but same as before)
  } else {
    Serial.print("Checking MAX31856 Oil Temp: ");
    float t1 = max_oil.readCJTemperature();
    Serial.printf("CJ Temp: %.2f C\n", t1);
    Serial.println("✓ MAX31856 (Oil Temp) initialized");
    max_oil.setThermocoupleType(MAX31856_TCTYPE_K); // K-Type thermocouple
    max_oil.setConversionMode(MAX31856_CONTINUOUS);
    oilTempSensorFound = true;
  }

  // Initialize ADS1115 (Oil Pressure)
  if (!ads.begin(ADS1115_ADDR)) {
    Serial.println("✗ ADS1115 (Pressure) init failed! (Addr: 0x48)");
    pressureSensorFound = false;
  } else {
    Serial.println("✓ ADS1115 (Pressure) initialized");
    ads.setGain(GAIN_TWOTHIRDS); // +/- 6.144V
    pressureSensorFound = true;
  }

  // Initialize ESP-NOW
  if (!initESPNow()) {
    Serial.println("\n✗ ESP-NOW initialization failed!");
    while (1)
      delay(1000); // Halt
  }

  // Check receiver MAC address configuration
  bool macIsDefault = true;
  for (int i = 0; i < 6; i++) {
    if (receiverMAC[i] != 0xFF) {
      macIsDefault = false;
      break;
    }
  }

  if (macIsDefault) {
    Serial.println("\n⚠ WARNING: Receiver MAC address not configured!");
    Serial.println("  1. Flash the receiver code first");
    Serial.println("  2. Copy its MAC address from Serial Monitor");
    Serial.println("  3. Update receiverMAC[] in config.h");
    Serial.println("  4. Re-flash this sender code\n");
  } else {
    Serial.print("Receiver MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", receiverMAC[i]);
      if (i < 5)
        Serial.print(":");
    }
    Serial.println();
  }

  Serial.println("\n========================================");
  Serial.println("Starting temperature monitoring...");
  Serial.println("========================================\n");

  initConsole();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  handleConsole(); // Check for menu input
  unsigned long currentTime = millis();

  // Check if it's time to sample
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = currentTime;

    // Read Oil Temperature
    float oilTemp = 0;
    float oilCJ = 0;
    uint8_t oilFault = 0;
    if (oilTempSensorFound) {
      oilTemp = max_oil.readThermocoupleTemperature();
      oilCJ = max_oil.readCJTemperature();
      oilFault = max_oil.readFault();
      if (isnan(oilTemp) || oilFault != 0) {
        // Don't smooth faults, just pass invalid
        // If fault, display logic handles it.
      } else {
        // Apply Offset
        float adjustedTemp = oilTemp + SystemSettings.oilTempOffset;

        // Apply Smoothing (only if valid)
        if (currentOilTemperature == 0)
          currentOilTemperature = adjustedTemp; // Init
        oilTemp =
            (ALPHA * adjustedTemp) + ((1.0 - ALPHA) * currentOilTemperature);
      }
    }

    // Read Oil Pressure
    float pressurePSI = 0.0f;
    if (pressureSensorFound) {
      int16_t adc0 = ads.readADC_SingleEnded(0);
      float voltage = ads.computeVolts(adc0);

      // Clamp voltage to expected range (0.34V - 3.07V)
      if (voltage < SENSOR_MIN_VOLTAGE)
        voltage = SENSOR_MIN_VOLTAGE;
      if (voltage > SENSOR_MAX_VOLTAGE)
        voltage = SENSOR_MAX_VOLTAGE;

      // Calculate PSI
      pressurePSI = ((voltage - SENSOR_MIN_VOLTAGE) /
                     (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE)) *
                    SENSOR_MAX_PSI;

      pressurePSI += SystemSettings.oilPressOffset; // Apply Offset
    }

    currentOilPressure = pressurePSI;

    // Store current readings for display
    currentOilTemperature = oilTemp;
    currentOilColdJunction = oilCJ;
    currentOilFaultStatus = oilFault;

    dataValid = true;

    // Print to Serial only if menu is not active
    if (!isConsoleActive()) {
      Serial.println("----------------------------------------");
      Serial.printf("Seq: %d | Time: %lu ms\n", sequenceNumber, currentTime);
      Serial.printf("Oil: %.1f F | Press: %.1f PSI\n",
                    (currentOilTemperature * 1.8 + 32), currentOilPressure);

      // Detailed diagnostics
      Serial.println("--- OIL TEMP ---");
      if (hasFault(oilFault)) {
        Serial.printf("⚠ FAULT: %s (0x%02X)\n", getFaultDescription(oilFault),
                      oilFault);
      } else {
        Serial.printf("Temp: %.2f C\n", oilTemp);
      }
    }
  }

  // Check if it's time to transmit (independent of sampling)
  if (currentTime - lastTransmitTime >= TRANSMIT_INTERVAL_MS) {
    lastTransmitTime = currentTime;

    // Send via ESP-NOW using current values
    bool sent = sendTemperatureData(
        currentOilTemperature, currentOilColdJunction, currentOilFaultStatus);
    if (!sent) {
      if (!isConsoleActive())
        Serial.println("⚠ Failed to transmit data");
    }

    if (!isConsoleActive())
      Serial.println("----------------------------------------\n");
  }

  // Update display periodically
  if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
    lastDisplayUpdate = currentTime;
    updateDisplay();
  }

  // Small delay to prevent tight looping
  delay(10);
}
