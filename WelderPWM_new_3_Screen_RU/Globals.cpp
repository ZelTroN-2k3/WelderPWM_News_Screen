#include "Globals.h"

// Экран U8glib I2C 128x64
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST | U8G_I2C_OPT_NO_ACK);

// Кнопка управления
Button btn(switchPin);

// Настройки EEPROM
Settings settings;

// Переменные состояния
WorkMode workMode = modeMenu;
MenuPage currentMenuPage = PAGE_MAIN;
int menuCursor = 0;
bool isEditing = false;

// Пункты Главного Меню в PROGMEM
const char main_item_0[] PROGMEM = "1. Rezhim PET";
const char main_item_1[] PROGMEM = "2. Rezhim PETG";
const char main_item_2[] PROGMEM = "3. Rezhim PLA";
const char main_item_3[] PROGMEM = "4. Rezhim Cust";
const char main_item_4[] PROGMEM = "5. [ NASTROYKI ]";
const char* const mainMenuItems[MAIN_MENU_COUNT] PROGMEM = {
  main_item_0, main_item_1, main_item_2, main_item_3, main_item_4
};

// Пункты Меню Настроек в PROGMEM
const char set_item_0[] PROGMEM = "Nagrev PET";
const char set_item_1[] PROGMEM = "Nagrev PETG";
const char set_item_2[] PROGMEM = "Nagrev PLA";
const char set_item_3[] PROGMEM = "Nagrev Custom";
const char set_item_4[] PROGMEM = "Uderzhanie";
const char set_item_5[] PROGMEM = "Okhlazhdenie";
const char set_item_6[] PROGMEM = "Ventilyator";
const char set_item_7[] PROGMEM = "Temp Cely Cust";
const char set_item_8[] PROGMEM = "Vsego Svarok";
const char set_item_9[] PROGMEM = "Inversiya Ekr.";
const char set_item_10[] PROGMEM = "! Sbros Nastroek";
const char set_item_11[] PROGMEM = "< Sokhr. i Nazad";
const char* const settingsMenuItems[SETTINGS_MENU_COUNT] PROGMEM = {
  set_item_0, set_item_1, set_item_2, set_item_3, set_item_4, set_item_5,
  set_item_6, set_item_7, set_item_8, set_item_9, set_item_10, set_item_11
};

// Переменные таймеров
bool fanOn = false;
unsigned long workTimer = 0;
unsigned long blinkTimer = 0;
unsigned long iconAnimTimer = 0;
unsigned long selectedHeatingPeriod = 27000;
int selectedMaterialIdx = 0;
int currentSecLeft = -1;

// Переменные заставки
unsigned long lastActivityTime = 0;
bool saverActive = false;
bool sleepActive = false;
unsigned long animTimer = 0;
Spark sparks[NUM_SPARKS];
