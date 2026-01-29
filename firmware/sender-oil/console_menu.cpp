#include "console_menu.h"
#include "config.h"
#include "settings.h"
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MAX31856.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

extern Adafruit_MAX31856 max_oil; // Oil Temperature
extern Adafruit_ADS1115 ads;
extern Adafruit_ADS1115 ads;
extern uint8_t receiverMAC[];

// Helper to clear serial buffer
void clearSerialInput() {
  while (Serial.available())
    Serial.read();
}

// Helper to read a float from user input
float readFloat() {
  clearSerialInput();
  while (!Serial.available()) {
    delay(10);
  }
  float val = Serial.parseFloat();
  clearSerialInput();
  return val;
}

// Helper to read a byte/int
int readInt() {
  clearSerialInput();
  while (!Serial.available()) {
    delay(10);
  }
  int val = Serial.parseInt();
  clearSerialInput();
  return val;
}

void printMenu() {
  Serial.println("\n\n=== ENGINE MONITOR CONSOLE ===");
  Serial.println("[1] ESP-NOW Settings");
  Serial.println("[2] Device Status Check");
  Serial.println("[3] Temperature Sensors (Head/Oil)");
  Serial.println("[4] Oil Pressure Sensor");
  Serial.println("[5] Reset All Settings to Default");
  Serial.println("[q] Quit / Refresh Menu");
  Serial.print("Select > ");
}

void showESPNowMenu() {
  Serial.println("\n--- ESP-NOW SETTINGS ---");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Peer MAC: ");
  for (int i = 0; i < 6; i++)
    Serial.printf("%02X%s", receiverMAC[i], (i < 5) ? ":" : "");
  Serial.println();
  Serial.print("Channel: ");
  Serial.println(ESPNOW_CHANNEL);
  Serial.println("\n(Editing MAC/Channel requires code rebuild currently)");
  Serial.println("Press any key to return...");
  while (!Serial.available())
    delay(10);
  clearSerialInput();
}

void showDeviceStatus() {
  Serial.println("\n--- DEVICE STATUS ---");

  // I2C Scan
  Serial.println("Scanning I2C Bus...");
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("  I2C found at 0x%02X ", address);
      if (address == 0x3C)
        Serial.print("(OLED)");
      if (address == 0x48)
        Serial.print("(ADS1115)");
      Serial.println();
      nDevices++;
    }
  }
  if (nDevices == 0)
    Serial.println("  No I2C devices found.\n");
  else
    Serial.println("  I2C Scan Complete.\n");

  // SPI / Sensor Check
  Serial.print("Checking MAX31856 Oil Temp: ");
  float t1 = max_oil.readCJTemperature();
  if (isnan(t1))
    Serial.println("FAIL (Wiring? Clock?)");
  else
    Serial.printf("OK (Internal: %.2f C)\n", t1);

  // Oil Check Removed

  Serial.println("\nPress any key to return...");
  while (!Serial.available())
    delay(10);
  clearSerialInput();
}

void showTempMenu() {
  bool inSubMenu = true;
  while (inSubMenu) {
    float oilC = max_oil.readThermocoupleTemperature();
    float oilInt = max_oil.readCJTemperature();

    Serial.println("\n--- TEMPERATURE CONFIGUATION ---");
    Serial.printf("OIL: %.2f C | %.2f F (Int: %.2f C)\n",
                  oilC + SystemSettings.oilTempOffset,
                  (oilC + SystemSettings.oilTempOffset) * 1.8 + 32, oilInt);
    Serial.printf("     Offset: %.2f | Alarm > %.1f F\n",
                  SystemSettings.oilTempOffset,
                  SystemSettings.oilTempAlarmHigh);

    // Oil Temp Removed

    Serial.println("\n[1] Set Oil Temp Offset");
    Serial.println("[2] Set Oil Temp Alarm Limit");
    // Oil Options Removed
    Serial.println("[b] Back");
    Serial.print("Select > ");

    while (!Serial.available())
      delay(10);
    char c = Serial.read();
    clearSerialInput();

    if (c == 'b')
      inSubMenu = false;
    else if (c == '1') {
      Serial.print("Enter Oil Temp Offset (C): ");
      SystemSettings.oilTempOffset = readFloat();
      SystemSettings.save();
    } else if (c == '2') {
      Serial.print("Enter Oil Temp Alarm (F): ");
      SystemSettings.oilTempAlarmHigh = readFloat();
      SystemSettings.save();
    }
    // Oil handlers removed
  }
}

void showPressureMenu() {
  bool inSubMenu = true;
  while (inSubMenu) {
    int16_t adc = ads.readADC_SingleEnded(0);
    float volts = ads.computeVolts(adc);

    // Voltage Divider Logic Re-calc for display
    float sensorVoltage = volts;
    // Note: The main loop logic clamps this. Here we show RAW voltage to help
    // debug.

    // Calculate PSI
    float clampedVolts = volts;
    if (clampedVolts < SENSOR_MIN_VOLTAGE)
      clampedVolts = SENSOR_MIN_VOLTAGE;
    if (clampedVolts > SENSOR_MAX_VOLTAGE)
      clampedVolts = SENSOR_MAX_VOLTAGE;

    float psi = ((clampedVolts - SENSOR_MIN_VOLTAGE) /
                 (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE)) *
                SENSOR_MAX_PSI;
    psi += SystemSettings.oilPressOffset;

    Serial.println("\n--- OIL PRESSURE CONFIGURATION ---");
    Serial.printf("Raw ADC: %d\n", adc);
    Serial.printf("Voltage: %.3f V (Expected range: %.2f - %.2f)\n", volts,
                  SENSOR_MIN_VOLTAGE, SENSOR_MAX_VOLTAGE);
    Serial.printf("Calculated: %.1f PSI\n", psi);
    Serial.printf("Settings: Offset %.1f | Low < %.1f | High > %.1f\n",
                  SystemSettings.oilPressOffset,
                  SystemSettings.oilPressAlarmLow,
                  SystemSettings.oilPressAlarmHigh);

    Serial.println("\n[1] Set PSI Offset");
    Serial.println("[2] Set Low Alarm");
    Serial.println("[3] Set High Alarm");
    Serial.println("[b] Back");
    Serial.printf("Select > ");

    // Simple loop to refresh live readings if no input
    unsigned long t = millis();
    while (!Serial.available()) {
      if (millis() - t > 1000) { // Refresh reading every second
        // Just break loop to reprint menu with new values
        goto refresh_view;
      }
      delay(10);
    }

    char c;
    c = Serial.read();
    clearSerialInput();

    if (c == 'b')
      inSubMenu = false;
    else if (c == '1') {
      Serial.print("Enter PSI Offset: ");
      SystemSettings.oilPressOffset = readFloat();
      SystemSettings.save();
    } else if (c == '2') {
      Serial.print("Enter Low Alarm: ");
      SystemSettings.oilPressAlarmLow = readFloat();
      SystemSettings.save();
    } else if (c == '3') {
      Serial.print("Enter High Alarm: ");
      SystemSettings.oilPressAlarmHigh = readFloat();
      SystemSettings.save();
    }

  refresh_view:;
  }
}

static bool menuMode = false;

bool isConsoleActive() { return menuMode; }

void initConsole() { Serial.println("Press 'm' to enter menu..."); }

void handleConsole() {
  if (Serial.available()) {
    char c = Serial.read();

    // Ignore newline/cr unless it's the only input to wake up?
    // actually, let's keep it simple.
    if (c == '\n' || c == '\r')
      return;

    if (!menuMode) {
      // If not in menu mode, any key (e.g. 'm') enters menu mode
      menuMode = true;
      printMenu();
      return;
    }

    // In Menu Mode
    switch (c) {
    case '1':
      showESPNowMenu();
      printMenu();
      break;
    case '2':
      showDeviceStatus();
      printMenu();
      break;
    case '3':
      showTempMenu();
      printMenu();
      break;
    case '4':
      showPressureMenu();
      printMenu();
      break;
    case '5':
      Serial.println("Resetting to Defaults...");
      SystemSettings.resetDefaults();
      printMenu();
      break;
    case 'q':
    case 'x':
      Serial.println("Exiting Menu. Resuming Data Log...");
      menuMode = false;
      break;
    case 'm':
      printMenu(); // Redraw
      break;
    default:
      Serial.println("Unknown command. 'q' to quit.");
      printMenu();
      break;
    }
  }
}
