#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include "Globals.h"

// Spark initialization with random position and velocity
void initSparks();

// Update spark particles physics
void updateSparks();

// Render animated spark screen saver
void drawSparkScreenSaver();

#endif // SCREENSAVER_H
