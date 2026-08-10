#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Версия прошивки
#define FIRMWARE_VERSION "v3.8 Alpha"

// Отладка через Serial (0 = выключено, 1 = включено)
#define ENABLE_SERIAL_DEBUG 0

// Опциональный зуммер на пине D4 (0 = выключено, 1 = включено)
#define ENABLE_BUZZER 0

// --- Назначение пинов (Arduino Nano V3) ---
#define switchPin 10  // Пин кнопки
#define mosfetPin 3   // Пин нагревателя (ШИМ / PWM)
#define fanPin 2      // Пин вентилятора
#define redPin 8      // Пин красного светодиода
#define greenPin 6    // Пин зеленого светодиода
#define buzzerPin 4   // Опциональный пин зуммера
#define ntcPin A0     // Аналоговый пин для датчика температуры NTC 100k

// --- Параметры датчика температуры NTC 100k (B3950) ---
#define NTC_R_SERIES 100000.0f     // Подтягивающий резистор (100 кОм)
#define NTC_R_NOMINAL 100000.0f    // Номинальное сопротивление при 25°C (100 кОм)
#define NTC_TEMP_NOMINAL 25.0f     // Номинальная температура 25°C
#define NTC_BETA 3950.0f           // Коэффициент Бета 3950
#define NTC_MAX_SAFE_TEMP 295      // Максимальная аварийная температура (°C)
#define COOLDOWN_TARGET_TEMP 45    // Целевая температура отключения вентилятора (°C)

// --- Целевые температуры материалов (°C) ---
#define TARGET_TEMP_PET 280
#define TARGET_TEMP_PETG 230
#define TARGET_TEMP_PLA 190

// --- Значения ШИМ нагрева ---
#define heatNominalPWM 23
#define heatBoostPWM 38
#define heatBoostDuration 2000UL // 2 секунды начального форсированного нагрева
#define warmValue 10

// --- Периоды мигания светодиодов ---
#define warmingBlinkPeriod 500
#define coolingBlinkPeriod 500

// --- Таймауты спящего режима ---
#define SAVER_TIMEOUT 60000UL   // 1 минута заставки
#define SLEEP_TIMEOUT 120000UL  // 2 минуты до выключения экрана

// --- Настройки EEPROM ---
#define EEPROM_ADDRESS 0
#define EEPROM_MAGIC 0x5746

// Структура настроек в EEPROM
struct Settings {
  uint16_t magicHeader;      // Ключ идентификации (0x5746)
  uint16_t heatingPET;       // Таймаут безопасности PET (в секундах)
  uint16_t heatingPETG;      // Таймаут безопасности PETG (в секундах)
  uint16_t heatingPLA;       // Таймаут безопасности PLA (в секундах)
  uint16_t heatingCustom;    // Таймаут безопасности Custom (в секундах)
  uint16_t warmingPeriod;    // Время удержания (в секундах)
  uint16_t coolingPeriod;    // Время охлаждения (в секундах)
  uint16_t fanPeriod;        // Время работы вентилятора (в секундах)
  uint16_t tempTargetCustom; // Целевая температура Custom в °C (напр. 200°C)
  uint16_t weldCounter;      // Общее количество сварок
  uint16_t screenInverted;   // Инверсия экрана (0 = Черный фон, 1 = Белый фон)
};

// --- Режимы работы ---
enum WorkMode {
  modeMenu,           // Режим меню
  modeEditParam,      // Режим редактирования
  heatingMode,        // Режим нагрева
  warmingMode,        // Режим удержания
  coolingMode,        // Режим охлаждения
  forcedCoolMode,     // Принудительное охлаждение (30s)
  waitMode,           // Ожидание после сварки
  errorMode           // Режим тепловой аварии / ошибки
};

// --- Страницы меню ---
enum MenuPage {
  PAGE_MAIN,          // Главное меню
  PAGE_SETTINGS       // Меню настроек
};

#define MAIN_MENU_COUNT 5
#define SETTINGS_MENU_COUNT 12
#define NUM_SPARKS 25

// Частицы для заставки
struct Spark {
  int x;
  int y;
  int speedY;
  int speedX;
};

// Макросы отладки Serial
#if ENABLE_SERIAL_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
