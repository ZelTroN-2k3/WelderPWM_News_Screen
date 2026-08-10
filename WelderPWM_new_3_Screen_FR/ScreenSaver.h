#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include "Globals.h"

// Initialisation des étincelles avec des positions et vitesses aléatoires
void initSparks();

// Mise à jour de la physique des étincelles
void updateSparks();

// Rendu graphique de la pluie d'étincelles
void drawSparkScreenSaver();

#endif // SCREENSAVER_H
