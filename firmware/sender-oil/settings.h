#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

class Settings {
public:
  float headTempOffset;
  float headTempAlarmHigh;

  float oilTempOffset;
  float oilTempAlarmHigh;

  float oilPressOffset;
  float oilPressAlarmLow;
  float oilPressAlarmHigh;

  // ESP-NOW settings could be stored here too, but for now we focus on sensors

  void begin();
  void load();
  void save();
  void resetDefaults();

private:
  Preferences prefs;
};

extern Settings SystemSettings;

#endif
