#include "Hardware.h"

// Commande matérielle d'inversion des couleurs écran (SSD1309 / SSD1306 / SH1106)
void applyDisplayInversion(bool inverted) {
  Wire.beginTransmission(0x3C);
  Wire.write(0x00); // Mode Commande
  Wire.write(inverted ? 0xA7 : 0xA6); // 0xA7: Inversé (Fond Blanc), 0xA6: Normal (Fond Noir)
  Wire.endTransmission();

  Wire.beginTransmission(0x3D);
  Wire.write(0x00);
  Wire.write(inverted ? 0xA7 : 0xA6);
  Wire.endTransmission();
}

// Émission sonore Buzzer
void triggerBuzzer(int freq, int durationMs) {
#if ENABLE_BUZZER
  tone(buzzerPin, freq, durationMs);
#endif
}

// Réinitialisation des paramètres aux valeurs d'usine
void resetFactorySettings() {
  settings.magicHeader = EEPROM_MAGIC;
  settings.heatingPET = 35;     // 35s timeout sécurité
  settings.heatingPETG = 25;    // 25s timeout sécurité
  settings.heatingPLA = 25;     // 25s timeout sécurité
  settings.heatingCustom = 30;  // 30s timeout sécurité
  settings.warmingPeriod = 5;   // 5s
  settings.coolingPeriod = 20;  // 20s
  settings.fanPeriod = 300;     // 300s (5 min)
  settings.tempTargetCustom = 200; // 200°C cible Custom par défaut
  settings.weldCounter = 0;     // Compteur initial à 0
  settings.screenInverted = 0;  // Thème normal (Fond noir)
  saveSettings();
  applyDisplayInversion(0);
}

// Chargement des paramètres depuis l'EEPROM
void loadSettings() {
  EEPROM.get(EEPROM_ADDRESS, settings);
  // Si première utilisation ou mémoire vierge, charger les valeurs par défaut
  if (settings.magicHeader != EEPROM_MAGIC) {
    resetFactorySettings();
  } else {
    applyDisplayInversion(settings.screenInverted);
  }
}

// Sauvegarde des paramètres dans l'EEPROM
void saveSettings() {
  EEPROM.put(EEPROM_ADDRESS, settings);
}

// Obtention du pointeur vers la valeur du menu réglage
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

// Clignotement non bloquant de LED
void blinkLED(int pin, int blinkPeriod) {
  static int LED = 0;
  if (millis() - blinkTimer >= (unsigned long)blinkPeriod) {
    LED = !LED;
    digitalWrite(pin, LED ? HIGH : LOW);
    blinkTimer = millis();
  }
}

// Mesure et calcul de la température réelle de la sonde CTN 100k (°C)
float readTemperature() {
  long rawSum = 0;
  for (int i = 0; i < 8; i++) {
    rawSum += analogRead(ntcPin);
    delayMicroseconds(100);
  }
  float rawAdc = (float)rawSum / 8.0f;

  // Détection court-circuit (proche de 0) ou circuit ouvert (proche de 1023)
  if (rawAdc >= 1018.0f || rawAdc <= 5.0f) {
    return -999.0f; // Code d'erreur de lecture sonde
  }

  // Calcul résistance de la CTN (Pont diviseur : +5V -> R_SERIES -> A0 -> CTN -> GND)
  float rNtc = NTC_R_SERIES * rawAdc / (1023.0f - rawAdc);

  // Équation Steinhart-Hart avec paramètre Bêta
  float steinhart = rNtc / NTC_R_NOMINAL;            // (R/Ro)
  steinhart = log(steinhart);                        // ln(R/Ro)
  steinhart /= NTC_BETA;                             // 1/B * ln(R/Ro)
  steinhart += 1.0f / (NTC_TEMP_NOMINAL + 273.15f);   // + (1/To)
  steinhart = 1.0f / steinhart;                      // Inversion -> Kelvin
  float tempC = steinhart - 273.15f;                 // Conversion en degrés Celsius

  return tempC;
}

// Obtention de la température cible selon le mode
int getTargetTemperature(int modeIdx) {
  switch (modeIdx) {
    case 0: return TARGET_TEMP_PET;
    case 1: return TARGET_TEMP_PETG;
    case 2: return TARGET_TEMP_PLA;
    case 3: return settings.tempTargetCustom;
    default: return 200;
  }
}

// Vérification si la lecture de température est en erreur ou anormale
bool isTempSensorError(float temp) {
  return (temp < -10.0f || temp > (float)NTC_MAX_SAFE_TEMP);
}
