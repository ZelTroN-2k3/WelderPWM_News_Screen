#include "Globals.h"

// Instanciation U8glib pour écran OLED I2C 128x64 (SSD1309 / SSD1306 / SH1106 compatible)
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST | U8G_I2C_OPT_NO_ACK);

// Instanciation du bouton unique
Button btn(switchPin);

// Instance des réglages EEPROM
Settings settings;

// Variables d'état
WorkMode workMode = modeMenu;
MenuPage currentMenuPage = PAGE_MAIN;
int menuCursor = 0;
bool isEditing = false;

// Libellés du Menu Principal en PROGMEM
const char main_item_0[] PROGMEM = "1. Mode PET";
const char main_item_1[] PROGMEM = "2. Mode PETG";
const char main_item_2[] PROGMEM = "3. Mode PLA";
const char main_item_3[] PROGMEM = "4. Mode Custom";
const char main_item_4[] PROGMEM = "5. [ REGLAGES ]";
const char* const mainMenuItems[MAIN_MENU_COUNT] PROGMEM = {
  main_item_0, main_item_1, main_item_2, main_item_3, main_item_4
};

// Libellés du Menu Réglages en PROGMEM
const char set_item_0[] PROGMEM = "Chauffe PET";
const char set_item_1[] PROGMEM = "Chauffe PETG";
const char set_item_2[] PROGMEM = "Chauffe PLA";
const char set_item_3[] PROGMEM = "Chauffe Custom";
const char set_item_4[] PROGMEM = "Temps Maintien";
const char set_item_5[] PROGMEM = "Temps Refroid.";
const char set_item_6[] PROGMEM = "Temps Ventilo";
const char set_item_7[] PROGMEM = "Temp. Custom";
const char set_item_8[] PROGMEM = "Total Soudures";
const char set_item_9[] PROGMEM = "Invers. Ecran";
const char set_item_10[] PROGMEM = "! Reset Usine";
const char set_item_11[] PROGMEM = "< Sauver & Retour";
const char* const settingsMenuItems[SETTINGS_MENU_COUNT] PROGMEM = {
  set_item_0, set_item_1, set_item_2, set_item_3, set_item_4, set_item_5,
  set_item_6, set_item_7, set_item_8, set_item_9, set_item_10, set_item_11
};

// Variables d'exécution et timers
bool fanOn = false;
unsigned long workTimer = 0;
unsigned long blinkTimer = 0;
unsigned long iconAnimTimer = 0;
unsigned long selectedHeatingPeriod = 27000;
int selectedMaterialIdx = 0;
int currentSecLeft = -1;

// Variables pour la veille et les étincelles
unsigned long lastActivityTime = 0;
bool saverActive = false;
bool sleepActive = false;
unsigned long animTimer = 0;
Spark sparks[NUM_SPARKS];
