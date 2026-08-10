#include "Hardware.h"

// Команда инверсии цветов экрана (SSD1309 / SSD1306 / SH1106)
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

// Звуковой сигнал зуммера
void triggerBuzzer(int freq, int durationMs) {
#if ENABLE_BUZZER
  tone(buzzerPin, freq, durationMs);
#endif
}

// Сброс настроек на заводские
void resetFactorySettings() {
  settings.magicHeader = EEPROM_MAGIC;
  settings.heatingPET = 35;     // 35s таймаут безопасности
  settings.heatingPETG = 25;    // 25s таймаут безопасности
  settings.heatingPLA = 25;     // 25s таймаут безопасности
  settings.heatingCustom = 30;  // 30s таймаут безопасности
  settings.warmingPeriod = 5;   // 5s
  settings.coolingPeriod = 20;  // 20s
  settings.fanPeriod = 300;     // 300s (5 min)
  settings.tempTargetCustom = 200; // 200°C целевая Custom
  settings.weldCounter = 0;     // Начальный счетчик
  settings.screenInverted = 0;  // Обычная тема (Черный фон)
  saveSettings();
  applyDisplayInversion(0);
}

// Загрузка настроек из EEPROM
void loadSettings() {
  EEPROM.get(EEPROM_ADDRESS, settings);
  if (settings.magicHeader != EEPROM_MAGIC) {
    resetFactorySettings();
  } else {
    applyDisplayInversion(settings.screenInverted);
  }
}

// Сохранение настроек в EEPROM
void saveSettings() {
  EEPROM.put(EEPROM_ADDRESS, settings);
}

// Получение указателя на значение настройки
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

// Неблокирующее мигание светодиода
void blinkLED(int pin, int blinkPeriod) {
  static int LED = 0;
  if (millis() - blinkTimer >= (unsigned long)blinkPeriod) {
    LED = !LED;
    digitalWrite(pin, LED ? HIGH : LOW);
    blinkTimer = millis();
  }
}

// Измерение и вычисление температуры датчика NTC 100k (°C)
float readTemperature() {
  long rawSum = 0;
  for (int i = 0; i < 8; i++) {
    rawSum += analogRead(ntcPin);
    delayMicroseconds(100);
  }
  float rawAdc = (float)rawSum / 8.0f;

  if (rawAdc >= 1018.0f || rawAdc <= 5.0f) {
    return -999.0f;
  }

  float rNtc = NTC_R_SERIES * rawAdc / (1023.0f - rawAdc);

  float steinhart = rNtc / NTC_R_NOMINAL;
  steinhart = log(steinhart);
  steinhart /= NTC_BETA;
  steinhart += 1.0f / (NTC_TEMP_NOMINAL + 273.15f);
  steinhart = 1.0f / steinhart;
  float tempC = steinhart - 273.15f;

  return tempC;
}

// Получение целевой температуры для режима
int getTargetTemperature(int modeIdx) {
  switch (modeIdx) {
    case 0: return TARGET_TEMP_PET;
    case 1: return TARGET_TEMP_PETG;
    case 2: return TARGET_TEMP_PLA;
    case 3: return settings.tempTargetCustom;
    default: return 200;
  }
}

// Проверка на ошибку датчика температуры
bool isTempSensorError(float temp) {
  return (temp < -10.0f || temp > (float)NTC_MAX_SAFE_TEMP);
}
