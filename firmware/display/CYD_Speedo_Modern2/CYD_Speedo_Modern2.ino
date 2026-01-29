/*
 * CYD GPS Speedometer - Modern Design 2: DASHBOARD
 * Large speed with organized data panels
 * Modern dashboard layout with visual hierarchy
 *
 * Font Note: Uses built-in fonts at maximum size (8) for speed.
 * For even larger fonts, see Modern1 comments about downloading
 * FreeSansBold48pt7b or using truetype2gfx converter.
 */

#include <Adafruit_GFX.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <XPT2046_Touchscreen_TT.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ===== OIL SENDER DATA PACKET (Protocol v3) =====
typedef struct __attribute__((packed)) {
  uint8_t version;     // Protocol version = 3
  uint32_t timestamp;  // Milliseconds since boot
  float temperature;   // Head Temperature in Celsius (currently unused)
  float coldJunction;  // Head Cold junction temperature
  uint8_t faultStatus; // Head MAX31856 fault register

  float oilTemperature;   // Oil Temperature in Celsius
  float oilColdJunction;  // Oil Cold junction temperature
  uint8_t oilFaultStatus; // Oil MAX31856 fault register

  float oilPressure;       // Oil Pressure in PSI
  uint8_t sensorsStatus;   // Bitmask: Bit 0=Head, 1=Oil Temp, 2=Oil Press
  uint16_t sequenceNumber; // Packet sequence number
  uint8_t batteryLevel;    // Battery percentage 0-100
  uint8_t checksum;        // XOR checksum
} SensorData;

// ===== FUEL SENDER DATA PACKET (Protocol v1) =====
typedef struct __attribute__((packed)) {
  uint8_t version;           // Protocol version = 1
  uint16_t raw_resistance;   // Fuel sender resistance in 0.01 Ohm units
  uint8_t fuel_percent;      // Calculated fuel percentage (0-100%)
  uint8_t fault_status;      // Fault flags
  uint32_t timestamp;        // Milliseconds since boot
  uint16_t sequence_number;  // Packet counter
  uint8_t checksum;          // XOR checksum
} FuelDataPacket;

// Fault status flags
#define FUEL_FAULT_NONE 0x00
#define FUEL_FAULT_OPEN_CIRCUIT 0x01
#define FUEL_FAULT_SHORT_CIRCUIT 0x02
#define FUEL_FAULT_LOW_FUEL 0x04

SensorData receivedData;
FuelDataPacket fuelData;

TFT_eSPI tft = TFT_eSPI();

#define XPT2046_IRQ 36
#define XPT2046_CS 33
#define SD_CS 4
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// GPS data
char incoming[200];
byte incomingIdx = 0;
float currentSpeed = 0.0;
String currentFixStatus = "No Fix";
int currentSatellites = 0;
String currentLat = "--";
String currentLon = "--";
String currentAlt = "--";
float currentHeading = 0.0;

// ESP-NOW Sensor data - Oil Sender
float currentOilTemp = 0.0;
float currentOilPressure = 0.0;
unsigned long lastOilUpdate = 0;
bool oilDataValid = false;

// ESP-NOW Sensor data - Fuel Sender
uint8_t currentFuelPercent = 0;
uint8_t fuelFaultStatus = 0;
unsigned long lastFuelUpdate = 0;
bool fuelDataValid = false;

#define DATA_TIMEOUT_MS 5000  // Mark data as stale if no update for 5 seconds

// Previous values
float prevSpeed = -1.0;
String prevFixStatus = "";
int prevSatellites = -1;
String prevLat = "";
String prevLon = "";
String prevAlt = "";
float prevHeading = -1.0;
float prevOilTemp = -999.0;
float prevOilPressure = -999.0;
uint8_t prevFuelPercent = 255;

unsigned long lastUpdate = 0;
bool firstDraw = true;

// ===== MODERN DASHBOARD DESIGN CONFIG =====
// Color scheme - orange/amber theme
#define COLOR_BACKGROUND 0x0000   // Black
#define COLOR_PANEL_BG 0x18E3     // Dark grey
#define COLOR_ACCENT 0xFD20       // Orange
#define COLOR_SPEED_ZERO 0x630C   // Dark orange
#define COLOR_SPEED_NORMAL 0xFD20 // Orange
#define COLOR_SPEED_FAST 0xFFE0   // Yellow
#define COLOR_TEXT_PRIMARY TFT_WHITE
#define COLOR_TEXT_SECONDARY 0xAD55 // Grey
#define COLOR_GOOD TFT_GREEN
#define COLOR_WARNING TFT_YELLOW
#define COLOR_BAD TFT_RED

// Layout
#define HEADER_HEIGHT 30
#define SPEED_PANEL_Y 35
#define SPEED_PANEL_H 90
#define INFO_PANEL_Y 130
#define PANEL_MARGIN 5

// ESP-NOW Receive Callback (ESP32 Arduino Core 3.x)
// Handles both oil sender (v3) and fuel sender (v1) packets
void onDataReceive(const esp_now_recv_info *recv_info, const uint8_t *data,
                   int data_len) {
  if (data_len < 1) return;  // Minimum: version byte
  
  uint8_t packet_version = data[0];
  
  // ===== OIL SENDER PACKET (TempDataPacket, v3) =====
  if (packet_version == 3 && data_len == sizeof(SensorData)) {
    memcpy(&receivedData, data, sizeof(SensorData));

    // Update current values
    currentOilTemp = receivedData.oilTemperature;
    currentOilPressure = receivedData.oilPressure;
    lastOilUpdate = millis();
    oilDataValid = true;

    // Debug output
    Serial.print("[OIL] ESP-NOW from: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", recv_info->src_addr[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.print(" - Oil Temp: ");
    Serial.print(currentOilTemp);
    Serial.print("C, Oil Pressure: ");
    Serial.print(currentOilPressure);
    Serial.println(" PSI");
  }
  
  // ===== FUEL SENDER PACKET (FuelDataPacket, v1) =====
  else if (packet_version == 1 && data_len == sizeof(FuelDataPacket)) {
    memcpy(&fuelData, data, sizeof(FuelDataPacket));

    // Update current values
    currentFuelPercent = fuelData.fuel_percent;
    fuelFaultStatus = fuelData.fault_status;
    lastFuelUpdate = millis();
    fuelDataValid = true;

    // Debug output
    Serial.print("[FUEL] ESP-NOW from: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", recv_info->src_addr[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.print(" - Fuel: ");
    Serial.print(currentFuelPercent);
    Serial.print("%, Resistance: ");
    Serial.print(fuelData.raw_resistance / 100.0, 1);
    Serial.print(" Ohm, Faults: 0x");
    Serial.println(fuelFaultStatus, HEX);
  }
  
  // Unknown packet type
  else {
    Serial.print("[UNKNOWN] Packet version=0x");
    Serial.print(packet_version, HEX);
    Serial.print(" size=");
    Serial.println(data_len);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Longer delay to let serial stabilize

  Serial.println("\n\n\n=== CYD GPS Speedometer - Modern Design 2 ===");

  // Initialize WiFi in Station mode for ESP-NOW
  Serial.println(">>> Init WiFi for ESP-NOW...");
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.disconnect();
  delay(500);
  
  // Set WiFi channel to match sender (channel 1)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  Serial.println(">>> WiFi channel set to 1");

  Serial.print(">>> MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println(">>> COPY THIS MAC ADDRESS FOR YOUR SENDER!");

  // Initialize ESP-NOW
  Serial.println(">>> Init ESP-NOW...");
  delay(100);

  esp_err_t result = esp_now_init();
  if (result != ESP_OK) {
    Serial.print(">>> ERROR: ESP-NOW init failed! Error code: ");
    Serial.println(result);
  } else {
    Serial.println(">>> ESP-NOW initialized successfully");

    // Register receive callback
    esp_now_register_recv_cb(onDataReceive);
    Serial.println(">>> ESP-NOW receive callback registered");
    Serial.println(">>> Waiting for data from sender: 98:A3:16:8E:6A:E4 (ESP32-1)");
  }
  Serial.println();

  // Backlight
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  delay(100);

  // TFT
  Serial.println("Init TFT...");
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BACKGROUND);
  Serial.println("TFT ready");

  // Touch
  ts.begin();
  ts.setRotation(1);
  Serial.println("Touch ready");

  // SD Card
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  if (SD.begin(SD_CS)) {
    Serial.println("SD Card ready");
  }

  // Draw initial screen
  drawScreen();
  firstDraw = false;

  Serial.println("=== READY ===");
}

void loop() {
  // Read serial GPS data
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (incomingIdx > 0) {
        incoming[incomingIdx] = '\0';
        parseGPSData(incoming);
        updateScreen();
      }
      incomingIdx = 0;
    } else if (c != '\r' && incomingIdx < 199) {
      incoming[incomingIdx++] = c;
    }
  }

  if (millis() - lastUpdate > 2000) {
    updateScreen();
    lastUpdate = millis();
  }

  delay(10);
}

void parseGPSData(char *data) {
  int len = strlen(data);
  while (len > 0 && (data[len - 1] == '\n' || data[len - 1] == '\r')) {
    data[len - 1] = '\0';
    len--;
  }

  char *parts[7];
  byte partIdx = 0;
  parts[0] = data;

  for (int i = 0; i < len && partIdx < 6; i++) {
    if (data[i] == '|') {
      data[i] = '\0';
      parts[++partIdx] = &data[i + 1];
    }
  }

  if (partIdx >= 0 && parts[0][0] != '\0')
    currentSpeed = atof(parts[0]);
  if (partIdx >= 1 && parts[1][0] != '\0')
    currentFixStatus = String(parts[1]);
  if (partIdx >= 2 && parts[2] != NULL && strlen(parts[2]) > 0)
    currentLat = String(parts[2]);
  if (partIdx >= 3 && parts[3] != NULL && strlen(parts[3]) > 0)
    currentLon = String(parts[3]);
  if (partIdx >= 4 && parts[4] != NULL && strlen(parts[4]) > 0)
    currentAlt = String(parts[4]);
  if (partIdx >= 5 && parts[5] != NULL && strlen(parts[5]) > 0)
    currentHeading = atof(parts[5]);
  if (partIdx >= 6 && parts[6][0] != '\0')
    currentSatellites = atoi(parts[6]);

  lastUpdate = millis();
}

uint16_t getSpeedColor(float speed) {
  int speedInt = (int)round(speed);
  if (speedInt == 0)
    return COLOR_SPEED_ZERO;
  else if (speedInt >= 1 && speedInt <= 60)
    return COLOR_SPEED_NORMAL;
  else
    return COLOR_SPEED_FAST;
}

uint16_t getFixColor(String fixStatus) {
  if (fixStatus.indexOf("3D Fix") >= 0)
    return COLOR_GOOD;
  else if (fixStatus.indexOf("2D Fix") >= 0)
    return COLOR_WARNING;
  else
    return COLOR_BAD;
}

void updateScreen() {
  if (firstDraw) {
    drawScreen();
    firstDraw = false;
    return;
  }

  // Update speed if changed
  if (abs(currentSpeed - prevSpeed) > 0.1) {
    drawSpeedPanel();
    prevSpeed = currentSpeed;
  }

  // Update header if status changed
  if (currentFixStatus != prevFixStatus ||
      currentSatellites != prevSatellites) {
    drawHeader();
    prevFixStatus = currentFixStatus;
    prevSatellites = currentSatellites;
  }

  // Update info if changed
  if (currentLat != prevLat || currentLon != prevLon || currentAlt != prevAlt ||
      abs(currentHeading - prevHeading) > 0.1 ||
      abs(currentOilTemp - prevOilTemp) > 0.1 ||
      abs(currentOilPressure - prevOilPressure) > 0.1) {
    drawInfoPanels();
    prevLat = currentLat;
    prevLon = currentLon;
    prevAlt = currentAlt;
    prevHeading = currentHeading;
    prevOilTemp = currentOilTemp;
    prevOilPressure = currentOilPressure;
  }

  // Check if sensor data is stale (no update in 5 seconds)
  if (oilDataValid && (millis() - lastOilUpdate > DATA_TIMEOUT_MS)) {
    oilDataValid = false;
    drawInfoPanels(); // Redraw to show "No Data"
  }
  
  if (fuelDataValid && (millis() - lastFuelUpdate > DATA_TIMEOUT_MS)) {
    fuelDataValid = false;
    drawInfoPanels(); // Redraw to show "No Data"
  }
  
  // Check if fuel percent changed
  if (currentFuelPercent != prevFuelPercent) {
    drawInfoPanels();
    prevFuelPercent = currentFuelPercent;
  }
}

void drawScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  drawHeader();
  drawSpeedPanel();
  drawInfoPanels();
}

void drawHeader() {
  // Draw header bar
  tft.fillRect(0, 0, 320, HEADER_HEIGHT, COLOR_PANEL_BG);
  tft.drawFastHLine(0, HEADER_HEIGHT, 320, COLOR_ACCENT);

  // GPS Fix status - left side
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setFreeFont(NULL);
  uint16_t fixColor = getFixColor(currentFixStatus);

  // Draw fix indicator circle
  tft.fillCircle(10, 15, 6, fixColor);
  tft.drawString(currentFixStatus, 22, 8);

  // Satellites - right side
  tft.setTextDatum(TR_DATUM);
  uint16_t satColor = currentSatellites >= 5 ? COLOR_GOOD : COLOR_WARNING;
  tft.setTextColor(satColor, COLOR_PANEL_BG);

  // Draw satellite icon (simple)
  int satX = 265;
  tft.fillRect(satX, 12, 8, 8, satColor);
  tft.fillRect(satX + 2, 8, 4, 4, satColor);

  tft.drawString(String(currentSatellites) + " SAT", 310, 8);
}

void drawSpeedPanel() {
  // Draw speed panel background
  tft.fillRoundRect(PANEL_MARGIN, SPEED_PANEL_Y, 320 - PANEL_MARGIN * 2,
                    SPEED_PANEL_H, 8, COLOR_PANEL_BG);

  // Draw accent bar
  uint16_t speedColor = getSpeedColor(currentSpeed);
  tft.fillRoundRect(PANEL_MARGIN, SPEED_PANEL_Y, 320 - PANEL_MARGIN * 2, 4, 2,
                    speedColor);

  // Draw HUGE speed number (centered)
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(8);
  tft.setFreeFont(NULL);
  int speedRounded = (int)round(currentSpeed);
  tft.setTextColor(speedColor, COLOR_PANEL_BG);
  tft.drawString(String(speedRounded), 160, 75);

  // Draw MPH label
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
  tft.drawString("MPH", 160, 108);
}

void drawInfoPanels() {
  int panelY = INFO_PANEL_Y;
  int panelW = (320 - PANEL_MARGIN * 3) / 2;
  int panelH = 50;

  // Left panel - Coordinates
  tft.fillRoundRect(PANEL_MARGIN, panelY, panelW, panelH, 6, COLOR_PANEL_BG);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setFreeFont(NULL);
  tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
  tft.drawString("POSITION", PANEL_MARGIN + 5, panelY + 5);

  tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_PANEL_BG);
  String lat = currentLat.substring(0, min((int)currentLat.length(), 11));
  String lon = currentLon.substring(0, min((int)currentLon.length(), 11));
  tft.drawString(lat, PANEL_MARGIN + 5, panelY + 18);
  tft.drawString(lon, PANEL_MARGIN + 5, panelY + 30);

  // Right panel - Heading & Altitude
  int rightPanelX = PANEL_MARGIN * 2 + panelW;
  tft.fillRoundRect(rightPanelX, panelY, panelW, panelH, 6, COLOR_PANEL_BG);
  tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
  tft.drawString("HEADING", rightPanelX + 5, panelY + 5);

  tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_PANEL_BG);
  tft.drawString(String((int)currentHeading) + " deg", rightPanelX + 5,
                 panelY + 18);

  String alt = currentAlt;
  if (alt.length() > 8)
    alt = alt.substring(0, 8);
  tft.drawString("ALT " + alt, rightPanelX + 5, panelY + 30);

  // Draw compass visualization
  drawMiniCompass(rightPanelX + panelW - 35, panelY + 30, 18, currentHeading);

  // Bottom panel - Oil Temp, Oil Pressure, and Fuel Level
  panelY = INFO_PANEL_Y + panelH + PANEL_MARGIN;
  panelH = 240 - panelY - PANEL_MARGIN;
  tft.fillRoundRect(PANEL_MARGIN, panelY, 320 - PANEL_MARGIN * 2, panelH, 6,
                    COLOR_PANEL_BG);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setFreeFont(NULL);

  if (oilDataValid) {
    // Oil Temperature - Left side
    tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
    tft.drawString("OIL TEMP", PANEL_MARGIN + 10, panelY + 8);

    tft.setTextSize(2);
    tft.setTextColor(COLOR_ACCENT, COLOR_PANEL_BG);
    // Convert Celsius to Fahrenheit for display
    float tempF = (currentOilTemp * 9.0 / 5.0) + 32.0;
    String tempStr = String(tempF, 1) + " F";
    tft.drawString(tempStr, PANEL_MARGIN + 10, panelY + 22);

    // Oil Pressure - Middle
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
    tft.drawString("OIL PRESSURE", 120, panelY + 8);

    tft.setTextSize(2);
    // Color code pressure (warning if < 10 PSI, good if >= 10)
    uint16_t pressureColor =
        currentOilPressure >= 10.0 ? COLOR_GOOD : COLOR_WARNING;
    tft.setTextColor(pressureColor, COLOR_PANEL_BG);
    String pressureStr = String(currentOilPressure, 1) + " PSI";
    tft.drawString(pressureStr, 120, panelY + 22);
  } else {
    // No oil data
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
    tft.drawString("OIL: No Data", PANEL_MARGIN + 10, panelY + 8);
  }
  
  // Fuel Level - Right side
  if (fuelDataValid) {
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
    tft.drawString("FUEL", 220, panelY + 8);

    tft.setTextSize(2);
    // Color code fuel level (red if low < 15%, yellow if < 25%, green otherwise)
    uint16_t fuelColor = COLOR_GOOD;
    if (currentFuelPercent < 15) {
      fuelColor = COLOR_BAD;  // Red
    } else if (currentFuelPercent < 25) {
      fuelColor = COLOR_WARNING;  // Yellow
    }
    tft.setTextColor(fuelColor, COLOR_PANEL_BG);
    String fuelStr = String(currentFuelPercent) + "%";
    tft.drawString(fuelStr, 220, panelY + 22);
    
    // Fuel fault indicator
    if (fuelFaultStatus & ~FUEL_FAULT_NONE) {
      tft.setTextSize(1);
      tft.setTextColor(COLOR_BAD, COLOR_PANEL_BG);
      tft.drawString("FAULT!", 220, panelY + 38);
    }
  } else {
    // No fuel data
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_PANEL_BG);
    tft.drawString("FUEL: No Data", 220, panelY + 8);
  }
}

void drawMiniCompass(int x, int y, int radius, float heading) {
  // Draw circle
  tft.drawCircle(x, y, radius, COLOR_ACCENT);

  // Draw heading line
  float rad = heading * PI / 180.0;
  int x2 = x + (radius - 3) * sin(rad);
  int y2 = y - (radius - 3) * cos(rad);

  tft.drawLine(x, y, x2, y2, COLOR_ACCENT);
  tft.fillCircle(x2, y2, 2, COLOR_ACCENT);
}
