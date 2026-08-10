#ifndef GLOBALS_H
#define GLOBALS_H

#include "Config.h"
#include <U8glib.h>
#include <EncButton.h>
#include <EEPROM.h>
#include <Wire.h>

// Общие аппаратные объекты
extern U8GLIB_SSD1306_128X64 u8g;
extern Button btn;
extern Settings settings;

// Переменные состояния
extern WorkMode workMode;
extern MenuPage currentMenuPage;
extern int menuCursor;
extern bool isEditing;

// Пункты меню в Flash памяти (PROGMEM)
extern const char* const mainMenuItems[MAIN_MENU_COUNT] PROGMEM;
extern const char* const settingsMenuItems[SETTINGS_MENU_COUNT] PROGMEM;

// Переменные выполнения и таймеры
extern bool fanOn;
extern unsigned long workTimer;
extern unsigned long blinkTimer;
extern unsigned long iconAnimTimer;
extern unsigned long selectedHeatingPeriod;
extern int selectedMaterialIdx;
extern int currentSecLeft;

// Переменные спящего режима
extern unsigned long lastActivityTime;
extern bool saverActive;
extern bool sleepActive;
extern unsigned long animTimer;
extern Spark sparks[NUM_SPARKS];

#endif // GLOBALS_H
