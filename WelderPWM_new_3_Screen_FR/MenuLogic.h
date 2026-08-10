#ifndef MENULOGIC_H
#define MENULOGIC_H

#include "Globals.h"
#include "Hardware.h"
#include "Display.h"

// Gestion des clics et de la navigation dans le menu
void handleMenuNavigation();

// Fonctions de transition des phases
void switch2Heating();
void switch2Warming();
void switch2Cooling();
void switch2ForcedCooling();
void switch2Wait();
void switch2Emergency(const char* title, const char* reason);

#endif // MENULOGIC_H
