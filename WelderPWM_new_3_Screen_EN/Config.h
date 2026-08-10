#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Firmware Version
#define FIRMWARE_VERSION "v3.8 Alpha"

// Serial Monitor Debug (0 = disabled, 1 = enabled)
#define ENABLE_SERIAL_DEBUG 0

// Optional Piezo Buzzer on pin D4 (0 = disabled, 1 = enabled)
#define ENABLE_BUZZER 0

// --- Hardware Pin Assignment (Arduino Nano V3) ---
#define switchPin 10  // Button pin
#define mosfetPin 3   // Heater pin (PWM compatible)
#define fanPin 2      // Fan pin
#define redPin 8      // Red LED pin
#define greenPin 6    // Green LED pin
#define buzzerPin 4   // Optional Buzzer pin
#define ntcPin A0     // Analog pin for NTC 100k temperature sensor

// --- NTC 100k Temperature Sensor Parameters (NTC 100k B3950) ---
#define NTC_R_SERIES 100000.0f     // Pull-up resistor value (100 kOhms)
#define NTC_R_NOMINAL 100000.0f    // Thermistor nominal resistance at 25°C (100 kOhms)
#define NTC_TEMP_NOMINAL 25.0f     // Nominal temperature 25°C
#define NTC_BETA 3950.0f           // Beta coefficient 3950
#define NTC_MAX_SAFE_TEMP 295      // Max emergency cutoff temperature (°C)
#define COOLDOWN_TARGET_TEMP 45    // Target temperature to turn off fan (°C)

// --- Target Temperatures by Material (°C) ---
#define TARGET_TEMP_PET 280
#define TARGET_TEMP_PETG 230
#define TARGET_TEMP_PLA 190

// --- PWM Heating Values ---
#define heatNominalPWM 23
#define heatBoostPWM 38
#define heatBoostDuration 2000UL // 2 seconds initial boost
#define warmValue 10

// --- LED Blink Periods ---
#define warmingBlinkPeriod 500
#define coolingBlinkPeriod 500

// --- Sleep Timeouts ---
#define SAVER_TIMEOUT 60000UL   // 1 minute screen saver
#define SLEEP_TIMEOUT 120000UL  // 2 minutes display sleep

// --- EEPROM Configuration ---
#define EEPROM_ADDRESS 0
#define EEPROM_MAGIC 0x5746

// Settings structure stored in EEPROM
struct Settings {
  uint16_t magicHeader;      // EEPROM magic key (0x5746)
  uint16_t heatingPET;       // Safety timeout for PET (seconds)
  uint16_t heatingPETG;      // Safety timeout for PETG (seconds)
  uint16_t heatingPLA;       // Safety timeout for PLA (seconds)
  uint16_t heatingCustom;    // Safety timeout for Custom (seconds)
  uint16_t warmingPeriod;    // Warming/holding time (seconds)
  uint16_t coolingPeriod;    // Cooling time (seconds)
  uint16_t fanPeriod;        // Fan running time (seconds)
  uint16_t tempTargetCustom; // Custom mode target temperature in °C (e.g. 200°C)
  uint16_t weldCounter;      // Total successful welds counter
  uint16_t screenInverted;   // Color inversion (0 = Black BG, 1 = White BG)
};

// --- System Operating Modes ---
enum WorkMode {
  modeMenu,           // Menu navigation mode
  modeEditParam,      // Parameter editing mode
  heatingMode,        // Heating phase mode
  warmingMode,        // Temperature holding phase mode
  coolingMode,        // Cooling phase mode
  forcedCoolMode,     // Forced Cooling mode (30s)
  waitMode,           // Post-welding wait mode
  errorMode           // Thermal safety error mode
};

// --- Menu Pages ---
enum MenuPage {
  PAGE_MAIN,          // Main menu (Material select / Settings)
  PAGE_SETTINGS       // Settings menu
};

#define MAIN_MENU_COUNT 5
#define SETTINGS_MENU_COUNT 12
#define NUM_SPARKS 25

// Spark particles for screensaver
struct Spark {
  int x;
  int y;
  int speedY;
  int speedX;
};

// Serial Debug Macros
#if ENABLE_SERIAL_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
