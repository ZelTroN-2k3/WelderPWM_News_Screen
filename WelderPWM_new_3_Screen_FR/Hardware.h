#ifndef HARDWARE_H
#define HARDWARE_H

#include "Globals.h"

// Commande matérielle d'inversion des couleurs écran (SSD1309 / SSD1306 / SH1106)
void applyDisplayInversion(bool inverted);

// Émission sonore Buzzer
void triggerBuzzer(int freq, int durationMs);

// Réinitialisation des paramètres aux valeurs d'usine
void resetFactorySettings();

// Chargement des paramètres depuis l'EEPROM
void loadSettings();

// Sauvegarde des paramètres dans l'EEPROM
void saveSettings();

// Obtention du pointeur vers la valeur du menu réglage
uint16_t* getSettingsValuePtr(int index);

// Clignotement non bloquant de LED
void blinkLED(int pin, int blinkPeriod);

// Mesure et calcul de la température réelle de la sonde CTN (°C)
float readTemperature();

// Obtention de la température cible pour un mode donné
int getTargetTemperature(int modeIdx);

// Vérification si la lecture de température est en erreur
bool isTempSensorError(float temp);

#endif // HARDWARE_H
