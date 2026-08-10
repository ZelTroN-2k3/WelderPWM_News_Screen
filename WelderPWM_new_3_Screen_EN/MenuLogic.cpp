#include "MenuLogic.h"

// --- Menu Navigation Click Handling ---
void handleMenuNavigation() {
  if (!btn.hasClicks()) return;
  lastActivityTime = millis();

  int clicks = btn.getClicks();

  // --- MAIN MENU ---
  if (currentMenuPage == PAGE_MAIN) {
    if (clicks == 1) {
      menuCursor = (menuCursor + 1) % MAIN_MENU_COUNT;
      drawMenu();
    } 
    else if (clicks == 3) {
      switch2ForcedCooling();
    }
    else if (clicks == 2) {
      if (menuCursor == 4) {
        DEBUG_PRINTLN(F("[MENU] Open Settings Submenu"));
        currentMenuPage = PAGE_SETTINGS;
        menuCursor = 0;
        drawMenu();
      } else {
        selectedMaterialIdx = menuCursor;
        if (menuCursor == 0) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPET * 1000UL;
          DEBUG_PRINT(F("[EVENT] Double-click -> Mode 1 (PET ~280°C) : "));
          DEBUG_PRINT(settings.heatingPET);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 1) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPETG * 1000UL;
          DEBUG_PRINT(F("[EVENT] Double-click -> Mode 2 (PETG ~230°C) : "));
          DEBUG_PRINT(settings.heatingPETG);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 2) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPLA * 1000UL;
          DEBUG_PRINT(F("[EVENT] Double-click -> Mode 3 (PLA ~190°C) : "));
          DEBUG_PRINT(settings.heatingPLA);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 3) {
          selectedHeatingPeriod = (unsigned long)settings.heatingCustom * 1000UL;
          DEBUG_PRINT(F("[EVENT] Double-click -> Mode 4 (Custom) : "));
          DEBUG_PRINT(settings.heatingCustom);
          DEBUG_PRINTLN(F(" s"));
        }

        switch2Heating();
      }
    }
  } 
  // --- SETTINGS MENU ---
  else if (currentMenuPage == PAGE_SETTINGS) {
    if (!isEditing) {
      if (clicks == 1) {
        menuCursor = (menuCursor + 1) % SETTINGS_MENU_COUNT;
        drawMenu();
      } 
      else if (clicks >= 2) {
        if (menuCursor == SETTINGS_MENU_COUNT - 1) {
          saveSettings();
          applyDisplayInversion(settings.screenInverted);
          DEBUG_PRINTLN(F("[MENU] Saved EEPROM & Return to Main Menu"));
          currentMenuPage = PAGE_MAIN;
          menuCursor = 0;
          drawMenu();
        } 
        else if (menuCursor == SETTINGS_MENU_COUNT - 2) {
          resetFactorySettings();
          DEBUG_PRINTLN(F("[MENU] Factory Reset Done !"));
          drawMessageScreen("Factory Reset", "Values restored!");
          delay(1500);
          drawMenu();
        } 
        else if (menuCursor == 8) {
          DEBUG_PRINT(F("[MENU] Total welds : "));
          DEBUG_PRINTLN(settings.weldCounter);
        }
        else {
          isEditing = true;
          DEBUG_PRINT(F("[MENU] Edit parameter : "));
          DEBUG_PRINTLN((const __FlashStringHelper*)pgm_read_word(&(settingsMenuItems[menuCursor])));
          drawMenu();
        }
      }
    } else {
      uint16_t* valPtr = getSettingsValuePtr(menuCursor);
      if (valPtr != NULL) {
        if (menuCursor == 9) {
          if (clicks == 1) {
            *valPtr = !(*valPtr);
            applyDisplayInversion(*valPtr);
            drawMenu();
          } 
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            applyDisplayInversion(*valPtr);
            DEBUG_PRINT(F("[MENU] Screen inversion : "));
            DEBUG_PRINTLN(*valPtr ? F("YES (White BG)") : F("NO (Black BG)"));
            drawMenu();
          }
        } 
        else if (menuCursor == 7) {
          if (clicks == 1) {
            *valPtr += 5;
            if (*valPtr > 290 || *valPtr < 150) *valPtr = 150;
            drawMenu();
          }
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            DEBUG_PRINT(F("[MENU] Custom target temp saved : "));
            DEBUG_PRINTLN(*valPtr);
            drawMenu();
          }
        }
        else {
          if (clicks == 1) {
            (*valPtr)++;
            if (*valPtr > 600) *valPtr = 1;
            drawMenu();
          } 
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            DEBUG_PRINT(F("[MENU] New value saved : "));
            DEBUG_PRINTLN(*valPtr);
            drawMenu();
          }
        }
      }
    }
  }
}

// --- Operating Mode Transitions ---

// Emergency thermal safety shutdown
void switch2Emergency(const char* title, const char* reason) {
  workMode = errorMode;
  analogWrite(mosfetPin, 0);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(fanPin, HIGH);
  fanOn = true;

  triggerBuzzer(800, 500);
  DEBUG_PRINT(F("\n[SAFETY ALERT] "));
  DEBUG_PRINT(title);
  DEBUG_PRINT(F(" - "));
  DEBUG_PRINTLN(reason);

  drawEmergencyScreen(title, reason);
}

// Phase 1 : Heating
void switch2Heating() {
  float currentTemp = readTemperature();
  if (isTempSensorError(currentTemp)) {
    switch2Emergency("NTC SENSOR ERROR", "Sensor disconnected");
    return;
  }

  workMode = heatingMode;
  workTimer = millis();
  iconAnimTimer = millis();

  int targetTemp = getTargetTemperature(selectedMaterialIdx);

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 1 : HEATING (Target : "));
  DEBUG_PRINT(targetTemp);
  DEBUG_PRINT(F("°C - Timeout : "));
  DEBUG_PRINT(selectedHeatingPeriod / 1000UL);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  digitalWrite(fanPin, LOW);
  analogWrite(mosfetPin, heatBoostPWM);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);

  triggerBuzzer(1500, 80);

  currentSecLeft = selectedHeatingPeriod / 1000UL;
  int progress = constrain(map((int)currentTemp, 25, targetTemp, 0, 100), 0, 100);
  char statusBuf[24];
  snprintf(statusBuf, sizeof(statusBuf), "Boost Heat : %dC", (int)currentTemp);
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, true, statusBuf);
}

// Phase 2 : Holding
void switch2Warming() {
  workMode = warmingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 2 : HOLDING TEMP (Duration : "));
  DEBUG_PRINT(settings.warmingPeriod);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  digitalWrite(redPin, LOW);

  triggerBuzzer(1800, 100);

  currentSecLeft = settings.warmingPeriod;
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, warmingMode, false, "MOVE <--|~|-->");
}

// Phase 3 : Cooling
void switch2Cooling() {
  workMode = coolingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 3 : COOLING (Duration : "));
  DEBUG_PRINT(settings.coolingPeriod);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  analogWrite(mosfetPin, 0);
  digitalWrite(fanPin, HIGH);
  digitalWrite(redPin, LOW);

  triggerBuzzer(1200, 100);

  currentSecLeft = settings.coolingPeriod;
  float currentTemp = readTemperature();
  char statusBuf[24];
  if (!isTempSensorError(currentTemp)) {
    snprintf(statusBuf, sizeof(statusBuf), "Cooling : %dC", (int)currentTemp);
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "Cooling...");
  }
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, coolingMode, false, statusBuf);
}

// Forced Cooling mode (30s)
void switch2ForcedCooling() {
  workMode = forcedCoolMode;
  workTimer = millis();
  iconAnimTimer = millis();
  fanOn = true;

  analogWrite(mosfetPin, 0);
  digitalWrite(fanPin, HIGH);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);

  triggerBuzzer(1600, 100);

  DEBUG_PRINTLN(F("\n[FORCED COOLING] Fan active for 30s"));
  currentSecLeft = 30;
  drawForcedCoolScreen(30, 0);
}

// Phase 4 : Post-welding completion
void switch2Wait() {
  workMode = waitMode;
  workTimer = millis();
  fanOn = true;

  settings.weldCounter++;
  saveSettings();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F(">>> PHASE 4 : WELD COMPLETED !"));
  DEBUG_PRINT(F("[STATS] Total welds completed : "));
  DEBUG_PRINTLN(settings.weldCounter);
  DEBUG_PRINT(F("[INFO] Fan remains active for "));
  DEBUG_PRINT(settings.fanPeriod);
  DEBUG_PRINTLN(F(" s"));
  DEBUG_PRINTLN(F("========================================"));

  analogWrite(mosfetPin, 0);
  digitalWrite(fanPin, HIGH);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);

  triggerBuzzer(2000, 120);

  drawWaitScreen();
}
