#include "MenuLogic.h"

// --- Gestion des clics dans le Menu ---
void handleMenuNavigation() {
  if (!btn.hasClicks()) return;
  lastActivityTime = millis();

  int clicks = btn.getClicks();

  // --- MENU PRINCIPAL ---
  if (currentMenuPage == PAGE_MAIN) {
    if (clicks == 1) {
      // Clic simple : déplacer le curseur
      menuCursor = (menuCursor + 1) % MAIN_MENU_COUNT;
      drawMenu();
    } 
    else if (clicks == 3) {
      // Triple clic : lancer le Refroidissement Forcé (30s)
      switch2ForcedCooling();
    }
    else if (clicks == 2) {
      // Clic double : valider l'élément
      if (menuCursor == 4) {
        DEBUG_PRINTLN(F("[MENU] Ouverture du sous-menu Réglages"));
        currentMenuPage = PAGE_SETTINGS;
        menuCursor = 0;
        drawMenu();
      } else {
        // Lancer la soudure pour le mode sélectionné
        selectedMaterialIdx = menuCursor;
        if (menuCursor == 0) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPET * 1000UL;
          DEBUG_PRINT(F("[EVENEMENT] Double-clic -> Lancement Mode 1 (PET ~280°C) : "));
          DEBUG_PRINT(settings.heatingPET);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 1) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPETG * 1000UL;
          DEBUG_PRINT(F("[EVENEMENT] Double-clic -> Lancement Mode 2 (PETG ~230°C) : "));
          DEBUG_PRINT(settings.heatingPETG);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 2) {
          selectedHeatingPeriod = (unsigned long)settings.heatingPLA * 1000UL;
          DEBUG_PRINT(F("[EVENEMENT] Double-clic -> Lancement Mode 3 (PLA ~190°C) : "));
          DEBUG_PRINT(settings.heatingPLA);
          DEBUG_PRINTLN(F(" s"));
        }
        else if (menuCursor == 3) {
          selectedHeatingPeriod = (unsigned long)settings.heatingCustom * 1000UL;
          DEBUG_PRINT(F("[EVENEMENT] Double-clic -> Lancement Mode 4 (Custom) : "));
          DEBUG_PRINT(settings.heatingCustom);
          DEBUG_PRINTLN(F(" s"));
        }

        switch2Heating();
      }
    }
  } 
  // --- MENU REGLAGES ---
  else if (currentMenuPage == PAGE_SETTINGS) {
    if (!isEditing) {
      if (clicks == 1) {
        // Deplacer le curseur
        menuCursor = (menuCursor + 1) % SETTINGS_MENU_COUNT;
        drawMenu();
      } 
      else if (clicks >= 2) {
        // Valider l'option sélectionnée
        if (menuCursor == SETTINGS_MENU_COUNT - 1) {
          // Sauvegarder & Retour au menu principal
          saveSettings();
          applyDisplayInversion(settings.screenInverted);
          DEBUG_PRINTLN(F("[MENU] Sauvegarde EEPROM & Retour au menu principal"));
          currentMenuPage = PAGE_MAIN;
          menuCursor = 0;
          drawMenu();
        } 
        else if (menuCursor == SETTINGS_MENU_COUNT - 2) {
          // Reset aux valeurs d'usine
          resetFactorySettings();
          DEBUG_PRINTLN(F("[MENU] Reinitialisation aux valeurs d'usine effectuee !"));
          drawMessageScreen("Reset Usine", "Valeurs restaurees!");
          delay(1500);
          drawMenu();
        } 
        else if (menuCursor == 8) {
          // Total Soudures (Informatif, lecture seule)
          DEBUG_PRINT(F("[MENU] Total soudures : "));
          DEBUG_PRINTLN(settings.weldCounter);
        }
        else {
          // Entrer en mode édition pour la valeur sélectionnée
          isEditing = true;
          DEBUG_PRINT(F("[MENU] Édition du paramètre : "));
          DEBUG_PRINTLN((const __FlashStringHelper*)pgm_read_word(&(settingsMenuItems[menuCursor])));
          drawMenu();
        }
      }
    } else {
      // Mode édition de valeur actif
      uint16_t* valPtr = getSettingsValuePtr(menuCursor);
      if (valPtr != NULL) {
        if (menuCursor == 9) {
          // Option Inversion Écran : bascule simple 0 <-> 1
          if (clicks == 1) {
            *valPtr = !(*valPtr);
            applyDisplayInversion(*valPtr);
            drawMenu();
          } 
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            applyDisplayInversion(*valPtr);
            DEBUG_PRINT(F("[MENU] Inversion ecran : "));
            DEBUG_PRINTLN(*valPtr ? F("OUI (Fond Blanc)") : F("NON (Fond Noir)"));
            drawMenu();
          }
        } 
        else if (menuCursor == 7) {
          // Option Température Cible Custom (Incrément par pas de 5°C de 150°C à 290°C)
          if (clicks == 1) {
            *valPtr += 5;
            if (*valPtr > 290 || *valPtr < 150) *valPtr = 150;
            drawMenu();
          }
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            DEBUG_PRINT(F("[MENU] Temp cible Custom sauvegardee : "));
            DEBUG_PRINTLN(*valPtr);
            drawMenu();
          }
        }
        else {
          // Paramètres de temps : incrément de 1s
          if (clicks == 1) {
            (*valPtr)++;
            if (*valPtr > 600) *valPtr = 1;
            drawMenu();
          } 
          else if (clicks >= 2) {
            isEditing = false;
            saveSettings();
            DEBUG_PRINT(F("[MENU] Nouvelle valeur sauvegardee : "));
            DEBUG_PRINTLN(*valPtr);
            drawMenu();
          }
        }
      }
    }
  }
}

// --- Fonctions de transition des modes de fonctionnement ---

// Arrêt d'urgence de sécurité thermique
void switch2Emergency(const char* title, const char* reason) {
  workMode = errorMode;
  analogWrite(mosfetPin, 0);   // Coupure immédiate du chauffage
  digitalWrite(redPin, HIGH);  // LED Rouge allumée
  digitalWrite(greenPin, LOW);
  digitalWrite(fanPin, HIGH);  // Ventilation forcée pour refroidir
  fanOn = true;

  triggerBuzzer(800, 500);
  DEBUG_PRINT(F("\n[ALERTE SECURITE] "));
  DEBUG_PRINT(title);
  DEBUG_PRINT(F(" - "));
  DEBUG_PRINTLN(reason);

  drawEmergencyScreen(title, reason);
}

// Phase 1 : Chauffe
void switch2Heating() {
  float currentTemp = readTemperature();
  if (isTempSensorError(currentTemp)) {
    switch2Emergency("ERREUR SONDE CTN", "Sonde deconnectee");
    return;
  }

  workMode = heatingMode;
  workTimer = millis();
  iconAnimTimer = millis();

  int targetTemp = getTargetTemperature(selectedMaterialIdx);

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 1 : CHAUFFE (Cible : "));
  DEBUG_PRINT(targetTemp);
  DEBUG_PRINT(F("°C - Timeout : "));
  DEBUG_PRINT(selectedHeatingPeriod / 1000UL);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  digitalWrite(fanPin, LOW);
  analogWrite(mosfetPin, heatBoostPWM); // Boost de chauffe initial
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);

  triggerBuzzer(1500, 80);

  currentSecLeft = selectedHeatingPeriod / 1000UL;
  int progress = constrain(map((int)currentTemp, 25, targetTemp, 0, 100), 0, 100);
  char statusBuf[24];
  snprintf(statusBuf, sizeof(statusBuf), "Chauffe Bst : %dC", (int)currentTemp);
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, true, statusBuf);
}

// Phase 2 : Maintien de température
void switch2Warming() {
  workMode = warmingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 2 : MAINTIEN TEMP (Duree : "));
  DEBUG_PRINT(settings.warmingPeriod);
  DEBUG_PRINTLN(F(" s)"));
  DEBUG_PRINTLN(F("========================================"));

  digitalWrite(redPin, LOW);

  triggerBuzzer(1800, 100);

  currentSecLeft = settings.warmingPeriod;
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, warmingMode, false, "BOUGER DE <--|~|-->");
}

// Phase 3 : Refroidissement
void switch2Cooling() {
  workMode = coolingMode;
  workTimer = millis();
  blinkTimer = millis();
  iconAnimTimer = millis();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F(">>> PHASE 3 : REFROIDISSEMENT (Duree : "));
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
    snprintf(statusBuf, sizeof(statusBuf), "Refroid. : %dC", (int)currentTemp);
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "Refroidissement");
  }
  drawMaterialScreen(selectedMaterialIdx, currentSecLeft, 0, coolingMode, false, statusBuf);
}

// Mode Refroidissement Forcé (30s)
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

  DEBUG_PRINTLN(F("\n[REFROIDISSEMENT FORCE] Ventilateur active pour 30s"));
  currentSecLeft = 30;
  drawForcedCoolScreen(30, 0);
}

// Phase 4 : Attente / Fin de soudure
void switch2Wait() {
  workMode = waitMode;
  workTimer = millis();
  fanOn = true;

  settings.weldCounter++;
  saveSettings();

  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F(">>> PHASE 4 : SOUDURE TERMINEE !"));
  DEBUG_PRINT(F("[STATS] Total soudures realisees : "));
  DEBUG_PRINTLN(settings.weldCounter);
  DEBUG_PRINT(F("[INFO] Ventilateur actif pour encore "));
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
