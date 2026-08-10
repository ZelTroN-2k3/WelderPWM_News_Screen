#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Version du Firmware
#define FIRMWARE_VERSION "v3.8 Alpha"

// Debug sur le moniteur Série (0 = désactivé, 1 = activé)
#define ENABLE_SERIAL_DEBUG 0

// Activation du Buzzer optionnel sur broche D4 (0 = désactivé, 1 = activé)
#define ENABLE_BUZZER 0

// --- Affectation des broches matérielles (Arduino Nano V3) ---
#define switchPin 10  // Broche du bouton
#define mosfetPin 3   // Broche du chauffage (compatible PWM)
#define fanPin 2      // Broche du ventilateur
#define redPin 8      // Broche de la LED rouge
#define greenPin 6    // Broche de la LED verte
#define buzzerPin 4   // Broche optionnelle pour Buzzer Piezo
#define ntcPin A0     // Broche analogique pour la sonde de température CTN 100k

// --- Paramètres de la sonde de température CTN (NTC 100k B3950) ---
#define NTC_R_SERIES 100000.0f     // Résistance de 100 kOhms
#define NTC_R_NOMINAL 100000.0f    // Sonde CTN 100 kOhms
#define NTC_TEMP_NOMINAL 25.0f     // Température nominale 25°C
#define NTC_BETA 3950.0f           // Coefficient Bêta 3950
#define NTC_MAX_SAFE_TEMP 295      // Température max de coupure d'urgence (°C)
#define COOLDOWN_TARGET_TEMP 45    // Température cible d'arrêt du ventilateur (°C)

// --- Températures cibles par matériau (°C) ---
#define TARGET_TEMP_PET 280
#define TARGET_TEMP_PETG 230
#define TARGET_TEMP_PLA 190

// --- Valeurs PWM pour la chauffe ---
#define heatNominalPWM 23
#define heatBoostPWM 38
#define heatBoostDuration 2000UL // 2 secondes de boost initial
#define warmValue 10

// --- Périodes de clignotement des LEDs ---
#define warmingBlinkPeriod 500
#define coolingBlinkPeriod 500

// --- Délais de mise en veille ---
#define SAVER_TIMEOUT 60000UL   // 1 minute pour l'économiseur d'écran
#define SLEEP_TIMEOUT 120000UL  // 2 minutes pour l'extinction totale de l'écran

// --- Configuration EEPROM ---
#define EEPROM_ADDRESS 0
#define EEPROM_MAGIC 0x5746

// Structure de configuration sauvegardée en EEPROM
struct Settings {
  uint16_t magicHeader;      // Clé d'identification pour l'EEPROM (0x5746)
  uint16_t heatingPET;       // Timeout sécurité chauffe PET (en secondes)
  uint16_t heatingPETG;      // Timeout sécurité chauffe PETG (en secondes)
  uint16_t heatingPLA;       // Timeout sécurité chauffe PLA (en secondes)
  uint16_t heatingCustom;    // Timeout sécurité chauffe Custom (en secondes)
  uint16_t warmingPeriod;    // Temps de maintien (en secondes)
  uint16_t coolingPeriod;    // Temps de refroidissement (en secondes)
  uint16_t fanPeriod;        // Temps de fonctionnement du ventilateur (en secondes)
  uint16_t tempTargetCustom; // Température cible Mode Custom en °C (ex: 200°C)
  uint16_t weldCounter;      // Compteur total de soudures réussies
  uint16_t screenInverted;   // Inversion des couleurs (0 = Fond Noir, 1 = Fond Blanc)
};

// --- États du système ---
enum WorkMode {
  modeMenu,           // Mode Navigation Menu
  modeEditParam,      // Mode Édition de Paramètre
  heatingMode,        // Mode de chauffe
  warmingMode,        // Mode de maintien de température
  coolingMode,        // Mode de refroidissement
  forcedCoolMode,     // Mode Refroidissement Forcé (30s)
  waitMode,           // Mode d'attente après soudure
  errorMode           // Mode Erreur / Sécurité Thermique
};

// --- États du Menu ---
enum MenuPage {
  PAGE_MAIN,          // Menu principal (Choix du matériau / Réglages)
  PAGE_SETTINGS       // Menu des réglages
};

#define MAIN_MENU_COUNT 5
#define SETTINGS_MENU_COUNT 12
#define NUM_SPARKS 25

// Structure pour les particules de l'économiseur d'écran
struct Spark {
  int x;
  int y;
  int speedY;
  int speedX;
};

// Macros pour le débogage Série
#if ENABLE_SERIAL_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
