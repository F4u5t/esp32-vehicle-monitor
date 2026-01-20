/*
 * Simple sketch to get ESP32 MAC Address
 * Upload this, open Serial Monitor at 115200 baud
 */

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== ESP32 MAC Address ===");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  String macAddress = WiFi.macAddress();

  Serial.println("Station Mode MAC Address:");
  Serial.println(macAddress);
  Serial.println(
      "\nUse this MAC address in your sender device's peer configuration!");
  Serial.println("========================\n");
}

void loop() {
  // Nothing to do here
  delay(1000);
}
