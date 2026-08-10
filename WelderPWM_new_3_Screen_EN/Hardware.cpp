#include "Hardware.h"

// Display color inversion hardware command (SSD1309 / SSD1306 / SH1106)
void applyDisplayInversion(bool inverted) {
  Wire.beginTransmission(0x3C);
  Wire.write(0x00);
  Wire.write(inverted ? 0xA7 : 0xA6);
  Wire.endTransmission();

  Wire.beginTransmission(0x3D);
  Wire.write(0x00);
  Wire.write(inverted ? 0xA7 : 0xA6);
  Wire.endTransmission();
}

// Piezo Buzzer tone emission
void triggerBuzzer(int freq, int durationMs) {
#if ENABLE_BUZZER
  tone(buzzerPin, freq, durationMs);
#endif
}

// Reset settings to factory defaults
void resetFactorySettings() {
  settings.magicHeader = EEPROM_MAGIC;
  settings.heatingPET = 35;     // 35s safety timeout
  settings.heatingPETG = 25;    // 25s safety timeout
  settings.heatingPLA = 25;     // 25s safety timeout
  settings.heatingCustom = 30;  // 30s safety timeout
  settings.warmingPeriod = 5;   // 5s
  settings.coolingPeriod = 20;  // 20s
  settings.fanPeriod = 300;     // 300s (5 min)
  settings.tempTargetCustom = 200; // 200°C default custom target
  settings.weldCounter = 0;     // Initial counter
  settings.screenInverted = 0;  // Normal theme (Black BG)
  saveSettings();
  applyDisplayInversion(0);
}

// Load settings from EEPROM
void loadSettings() {
  EEPROM.get(EEPROM_ADDRESS, settings);
  if (settings.magicHeader != EEPROM_MAGIC) {
    resetFactorySettings();
  } else {
    applyDisplayInversion(settings.screenInverted);
  }
}

// Save settings to EEPROM
void saveSettings() {
  EEPROM.put(EEPROM_ADDRESS, settings);
}

// Get settings parameter pointer
uint16_t* getSettingsValuePtr(int index) {
  switch (index) {
    case 0: return &settings.heatingPET;
    case 1: return &settings.heatingPETG;
    case 2: return &settings.heatingPLA;
    case 3: return &settings.heatingCustom;
    case 4: return &settings.warmingPeriod;
    case 5: return &settings.coolingPeriod;
    case 6: return &settings.fanPeriod;
    case 7: return &settings.tempTargetCustom;
    case 8: return &settings.weldCounter;
    case 9: return &settings.screenInverted;
    default: return NULL;
  }
}

// Non-blocking LED blink
void blinkLED(int pin, int blinkPeriod) {
  static int LED = 0;
  if (millis() - blinkTimer >= (unsigned long)blinkPeriod) {
    LED = !LED;
    digitalWrite(pin, LED ? HIGH : LOW);
    blinkTimer = millis();
  }
}

// Measure and compute NTC 100k temperature (°C)
float readTemperature() {
  long rawSum = 0;
  for (int i = 0; i < 8; i++) {
    rawSum += analogRead(ntcPin);
    delayMicroseconds(100);
  }
  float rawAdc = (float)rawSum / 8.0f;

  // Check open circuit or short circuit
  if (rawAdc >= 1018.0f || rawAdc <= 5.0f) {
    return -999.0f;
  }

  // Voltage divider formula
  float rNtc = NTC_R_SERIES * rawAdc / (1023.0f - rawAdc);

  // Steinhart-Hart equation with Beta parameter
  float steinhart = rNtc / NTC_R_NOMINAL;
  steinhart = log(steinhart);
  steinhart /= NTC_BETA;
  steinhart += 1.0f / (NTC_TEMP_NOMINAL + 273.15f);
  steinhart = 1.0f / steinhart;
  float tempC = steinhart - 273.15f;

  return tempC;
}

// Get target temperature according to mode
int getTargetTemperature(int modeIdx) {
  switch (modeIdx) {
    case 0: return TARGET_TEMP_PET;
    case 1: return TARGET_TEMP_PETG;
    case 2: return TARGET_TEMP_PLA;
    case 3: return settings.tempTargetCustom;
    default: return 200;
  }
}

// Check if temperature reading is abnormal
bool isTempSensorError(float temp) {
  return (temp < -10.0f || temp > (float)NTC_MAX_SAFE_TEMP);
}
