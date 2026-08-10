#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include "Globals.h"

// Инициализация частиц со случайными координатами и скоростью
void initSparks();

// Обновление физики частиц
void updateSparks();

// Отрисовка заставки
void drawSparkScreenSaver();

#endif // SCREENSAVER_H
