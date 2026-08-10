#include "MenuLogic.h"

// --- Обработка кликов в Меню ---
void handleMenuNavigation() {
  if (!btn.hasClicks()) return;
  lastActivityTime = millis();

  int clicks = btn.getClicks();

  // --- ГЛАВНОЕ МЕНЮ ---
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
        DEBUG_PRINTLN(F("[MENU] Otkrytie podmenyu Nastroek"));
        currentMenuPage = PAGE_SETTINGS;
        menuCursor = 0;
        drawMenu();
      } else {
        selectedMaterialIdx = menuCursor;
        if (menuCursor == 0) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPET * 1000UL;
          DEBUG_PRINT(F("[SOBYTIE] Dvoynoy klik -> Rezhim 1 (PET ~280°C) : "));
          DEBUG_PRINT(settings.heatingPET);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 1) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPETG * 1000UL;
          DEBUG_PRINT(F("[SOBYTIE] Dvoynoy klik -> Rezhim 2 (PETG ~230°C) : "));
          DEBUG_PRINT(settings.heatingPETG);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 2) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPLA * 1000UL;
          DEBUG_PRINT(F("[SOBYTIE] Dvoynoy klik -> Rezhim 3 (PLA ~190°C) : "));
          DEBUG_PRINT(settings.heatingPLA);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 3) {
          selectedHeatingPeriod = (unsigned long)settings.heatingCustom * 1000UL;
          DEBUG_PRINT(F("[SOBYTIE] Dvoynoy klik -> Rezhim 4 (Custom) : "));
          DEBUG_PRINT(settings.heatingCustom);
          DEBUG_PRINTLN(F(" s"));
        }

        switch2Heating();
      }
    }
  } 
  // --- МЕНЮ НАСТРОЕК ---
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
          DEBUG_PRINTLN(F("[MENU] Sokhranenie EEPROM i vykhod"));
          currentMenuPage = PAGE_MAIN;
          menuCursor = 0;
          drawMenu();
        } 
        else if (menuCursor == SETTINGS_MENU_COUNT - 2) {
          resetFactorySettings();
          DEBUG_PRINTLN(F("[MENU] Sbros na zavodskie nastroiki !"));
          drawMessageScreen("Sbros Usine", "Znacheniya sbrosheny!");
          delay(1500);
          drawMenu();
        } 
        else if (menuCursor == 8) {
          DEBUG_PRINT(F("[MENU] Vsego svarok : "));
          DEBUG_PRINTLN(settings.weldCounter);
        }
        else {
          isEditing = true;
          DEBUG_PRINT(F("[MENU] Redaktirovanie parametra : "));
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
            DEBUG_PRINT(F("[MENU] Inversiya ekrana : "));
            DEBUG_PRINTLN(*valPtr ? F("DA (Bely fon)") : F("NET (Cherny fon)"));
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
            DEBUG_PRINT(F("[MENU] Celevaya temp Custom : "));
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
            DEBUG_PRINT(F("[MENU] Novoe znachenie sokhraneno : "));
            DEBUG_PRINTLN(*valPtr);
            drawMenu();
          }
        }
      }
    }
  }
}

// --- Переходы между фазами ---

// Аварийное отключение нагрева
void switch2Emergency(const char* title, const char* reason) {
  workMode = errorMode;
  analogWrite(mosfetPin, 0);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(fanPin, HIGH);
  fanOn = true;

  triggerBuzzer(800, 500);
  DEBUG_PRINT(F("\n[AVARIYA] "));
  DEBUG_PRINT(title);
  DEBUG_PRINT(F(" - "));
  DEBUG_PRINTLN(reason);

  drawEmergencyScreen(title, reason);
}

// Фаза 1 : Нагрев
void switch2Heating() {
  float currentTemp = readTemperature();
  if (isTempSensorError(currentTemp)) {
    switch2Emergency("OSHIBKA DATCHIKA", "Datchik otkluchen");
    return;
  }

  workMode = heatingMode;
  workTimer = millis();
  iconAnimTimer = millis();

  int targetTemp = getTargetTemperature(selectedMaterialIdx);

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> FAZA 1 : NAGREV (Cel : "));
  DEBUG_PRINT(targetTemp);
  DEBUG_PRINT(F("°C - Taymaut : "));
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
  snprintf(statusBuf, sizeof(statusBuf), "Nagrev Bst : %dC", (int)currentTemp);
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, true, statusBuf);
}

// Фаза 2 : Удержание
void switch2Warming() {
  workMode = warmingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> FAZA 2 : UDERZHANIE (Dlitelnost : "));
  DEBUG_PRINT(settings.warmingPeriod);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  digitalWrite(redPin, LOW);

  triggerBuzzer(1800, 100);

  currentSecLeft = settings.warmingPeriod;
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, warmingMode, false, "DVIZHENIE <--|~|-->");
}

// Фаза 3 : Охлаждение
void switch2Cooling() {
  workMode = coolingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> FAZA 3 : OKHLAZHDENIE (Dlitelnost : "));
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
    snprintf(statusBuf, sizeof(statusBuf), "Okhlazhd. : %dC", (int)currentTemp);
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "Okhlazhdenie...");
  }
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, coolingMode, false, statusBuf);
}

// Режим принудительного охлаждения (30s)
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

  DEBUG_PRINTLN(F("\n[FORCED COOLING] Ventilyator vklyuchen na 30s"));
  currentSecLeft = 30;
  drawForcedCoolScreen(30, 0);
}

// Фаза 4 : Завершение сварки
void switch2Wait() {
  workMode = waitMode;
  workTimer = millis();
  fanOn = true;

  settings.weldCounter++;
  saveSettings();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F(">>> FAZA 4 : SVARKA ZAVERSHENA !"));
  DEBUG_PRINT(F("[STATS] Vsego svarok vypolneno : "));
  DEBUG_PRINTLN(settings.weldCounter);
  DEBUG_PRINT(F("[INFO] Ventilyator rabotaet eshche "));
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
