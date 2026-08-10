#ifndef DISPLAY_H
#define DISPLAY_H

#include "Globals.h"
#include "Hardware.h"

// Экран приветствия с рамкой HUD
void drawWelcomeScreen();

// Информационный экран
void drawMessageScreen(const char* line1, const char* line2);

// Анимация мини-иконок фазы
void drawPhaseIcon(int phaseMode, int frame, bool isBoost);

// Графический экран сварки
void drawMaterialScreen(int modeIdx, int secondsLeft, int progressPercent, int phaseMode, bool isBoost, const char* status1);

// Экран принудительного охлаждения (30s)
void drawForcedCoolScreen(int secLeft, int progress);

// Экран завершения сварки
void drawWaitScreen();

// Экран аварийной тепловой защиты
void drawEmergencyScreen(const char* title, const char* reason);

// Отрисовка динамического меню
void drawMenu();

#endif // DISPLAY_H
