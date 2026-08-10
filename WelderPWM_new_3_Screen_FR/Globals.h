#ifndef GLOBALS_H
#define GLOBALS_H

#include "Config.h"
#include <U8glib.h>
#include <EncButton.h>
#include <EEPROM.h>
#include <Wire.h>

// Objets matériels partagés
extern U8GLIB_SSD1306_128X64 u8g;
extern Button btn;
extern Settings settings;

// Variables d'état
extern WorkMode workMode;
extern MenuPage currentMenuPage;
extern int menuCursor;
extern bool isEditing;

// Libellés des menus en mémoire Flash (PROGMEM)
extern const char* const mainMenuItems[MAIN_MENU_COUNT] PROGMEM;
extern const char* const settingsMenuItems[SETTINGS_MENU_COUNT] PROGMEM;

// Variables temporelles et d'exécution
extern bool fanOn;
extern unsigned long workTimer;
extern unsigned long blinkTimer;
extern unsigned long iconAnimTimer;
extern unsigned long selectedHeatingPeriod;
extern int selectedMaterialIdx;
extern int currentSecLeft;

// Variables de gestion de veille
extern unsigned long lastActivityTime;
extern bool saverActive;
extern bool sleepActive;
extern unsigned long animTimer;
extern Spark sparks[NUM_SPARKS];

#endif // GLOBALS_H
