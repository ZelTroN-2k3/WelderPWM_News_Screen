#ifndef MENULOGIC_H
#define MENULOGIC_H

#include "Globals.h"
#include "Hardware.h"
#include "Display.h"

// Обработка кликов и навигация в меню
void handleMenuNavigation();

// Функции переходов между фазами
void switch2Heating();
void switch2Warming();
void switch2Cooling();
void switch2ForcedCooling();
void switch2Wait();
void switch2Emergency(const char* title, const char* reason);

#endif // MENULOGIC_H
