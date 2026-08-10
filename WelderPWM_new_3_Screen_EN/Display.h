#ifndef DISPLAY_H
#define DISPLAY_H

#include "Globals.h"
#include "Hardware.h"

// Welcome HUD startup screen
void drawWelcomeScreen();

// Ephemeral message screen
void drawMessageScreen(const char* line1, const char* line2);

// Draw animated mini-icons for each phase
void drawPhaseIcon(int phaseMode, int frame, bool isBoost);

// Structured material screen with countdown, target temp and progress bar
void drawMaterialScreen(int modeIdx, int secondsLeft, int progressPercent, int phaseMode, bool isBoost, const char* status1);

// Forced Cooling dedicated screen (30s) with animated fan
void drawForcedCoolScreen(int secLeft, int progress);

// Post-welding completion screen with counter
void drawWaitScreen();

// Thermal emergency safety alert screen
void drawEmergencyScreen(const char* title, const char* reason);

// Dynamic Main Menu and Settings Menu display
void drawMenu();

#endif // DISPLAY_H
