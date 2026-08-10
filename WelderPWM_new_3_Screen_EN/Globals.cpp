#include "Globals.h"

// U8glib instance for I2C OLED 128x64 display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST | U8G_I2C_OPT_NO_ACK);

// Single button instance
Button btn(switchPin);

// EEPROM settings instance
Settings settings;

// State variables
WorkMode workMode = modeMenu;
MenuPage currentMenuPage = PAGE_MAIN;
int menuCursor = 0;
bool isEditing = false;

// Main Menu labels in PROGMEM
const char main_item_0[] PROGMEM = "1. Mode PET";
const char main_item_1[] PROGMEM = "2. Mode PETG";
const char main_item_2[] PROGMEM = "3. Mode PLA";
const char main_item_3[] PROGMEM = "4. Mode Custom";
const char main_item_4[] PROGMEM = "5. [ SETTINGS ]";
const char* const mainMenuItems[MAIN_MENU_COUNT] PROGMEM = {
  main_item_0, main_item_1, main_item_2, main_item_3, main_item_4
};

// Settings Menu labels in PROGMEM
const char set_item_0[] PROGMEM = "Heat PET";
const char set_item_1[] PROGMEM = "Heat PETG";
const char set_item_2[] PROGMEM = "Heat PLA";
const char set_item_3[] PROGMEM = "Heat Custom";
const char set_item_4[] PROGMEM = "Warm Time";
const char set_item_5[] PROGMEM = "Cool Time";
const char set_item_6[] PROGMEM = "Fan Time";
const char set_item_7[] PROGMEM = "Custom Temp";
const char set_item_8[] PROGMEM = "Total Welds";
const char set_item_9[] PROGMEM = "Invert Screen";
const char set_item_10[] PROGMEM = "! Factory Reset";
const char set_item_11[] PROGMEM = "< Save & Return";
const char* const settingsMenuItems[SETTINGS_MENU_COUNT] PROGMEM = {
  set_item_0, set_item_1, set_item_2, set_item_3, set_item_4, set_item_5,
  set_item_6, set_item_7, set_item_8, set_item_9, set_item_10, set_item_11
};

// Execution and timer variables
bool fanOn = false;
unsigned long workTimer = 0;
unsigned long blinkTimer = 0;
unsigned long iconAnimTimer = 0;
unsigned long selectedHeatingPeriod = 27000;
int selectedMaterialIdx = 0;
int currentSecLeft = -1;

// Sleep and spark saver variables
unsigned long lastActivityTime = 0;
bool saverActive = false;
bool sleepActive = false;
unsigned long animTimer = 0;
Spark sparks[NUM_SPARKS];
