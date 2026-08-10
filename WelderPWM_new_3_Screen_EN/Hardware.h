#ifndef HARDWARE_H
#define HARDWARE_H

#include "Globals.h"

// Display color inversion hardware command
void applyDisplayInversion(bool inverted);

// Piezo Buzzer tone emission
void triggerBuzzer(int freq, int durationMs);

// Factory settings reset
void resetFactorySettings();

// Load settings from EEPROM
void loadSettings();

// Save settings to EEPROM
void saveSettings();

// Get settings parameter value pointer
uint16_t* getSettingsValuePtr(int index);

// Non-blocking LED blink
void blinkLED(int pin, int blinkPeriod);

// Measure and compute NTC 100k temperature (°C)
float readTemperature();

// Get target temperature for a given mode
int getTargetTemperature(int modeIdx);

// Check if temperature reading is an error
bool isTempSensorError(float temp);

#endif // HARDWARE_H
