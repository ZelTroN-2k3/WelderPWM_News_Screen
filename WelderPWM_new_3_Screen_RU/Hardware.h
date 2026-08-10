#ifndef HARDWARE_H
#define HARDWARE_H

#include "Globals.h"

// Команда инверсии цветов экрана
void applyDisplayInversion(bool inverted);

// Звуковой сигнал зуммера
void triggerBuzzer(int freq, int durationMs);

// Сброс настроек на заводские
void resetFactorySettings();

// Загрузка настроек из EEPROM
void loadSettings();

// Сохранение настроек в EEPROM
void saveSettings();

// Получение указателя на значение настройки
uint16_t* getSettingsValuePtr(int index);

// Неблокирующее мигание светодиода
void blinkLED(int pin, int blinkPeriod);

// Измерение и вычисление температуры датчика NTC 100k (°C)
float readTemperature();

// Получение целевой температуры для режима
int getTargetTemperature(int modeIdx);

// Проверка на ошибку датчика температуры
bool isTempSensorError(float temp);

#endif // HARDWARE_H
