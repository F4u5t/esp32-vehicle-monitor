#include "settings.h"

Settings SystemSettings;

void Settings::begin() {
  prefs.begin("car_mon", false); // "car_mon" is the namespace
  load();
}

void Settings::load() {
  headTempOffset = prefs.getFloat("h_off", 0.0f);
  headTempAlarmHigh = prefs.getFloat("h_lim", 220.0f); // Default 220F

  oilTempOffset = prefs.getFloat("o_off", 0.0f);
  oilTempAlarmHigh = prefs.getFloat("o_lim", 250.0f); // Default 250F

  oilPressOffset = prefs.getFloat("p_off", 0.0f);
  // Defaults: Warn if below 10 PSI or above 90 PSI
  oilPressAlarmLow = prefs.getFloat("p_lo_lim", 10.0f);
  oilPressAlarmHigh = prefs.getFloat("p_hi_lim", 90.0f);
}

void Settings::save() {
  prefs.putFloat("h_off", headTempOffset);
  prefs.putFloat("h_lim", headTempAlarmHigh);

  prefs.putFloat("o_off", oilTempOffset);
  prefs.putFloat("o_lim", oilTempAlarmHigh);

  prefs.putFloat("p_off", oilPressOffset);
  prefs.putFloat("p_lo_lim", oilPressAlarmLow);
  prefs.putFloat("p_hi_lim", oilPressAlarmHigh);
}

void Settings::resetDefaults() {
  headTempOffset = 0.0f;
  headTempAlarmHigh = 220.0f;
  oilTempOffset = 0.0f;
  oilTempAlarmHigh = 250.0f;
  oilPressOffset = 0.0f;
  oilPressAlarmLow = 10.0f;
  oilPressAlarmHigh = 90.0f;
  save();
}
