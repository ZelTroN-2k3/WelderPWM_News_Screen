/*
*|-----------------------------------------------------------------------------------------------------------|
*| Firmware        : Soudeuse de filament 3D automatisée (PWM) - Version Modulaire C++                       |
*| Auteur original : ptica (version v3.0)                                                                    |
*| Auteur évolutif : ZelTroN2k3 (version évoluée v3.8 Alpha)                                                 |
*| Matériel        : Arduino Nano V3 (ATmega328P) + Écran OLED I2C 128x64 (SSD1309 / SSD1306 / SH1106)       |
*|-----------------------------------------------------------------------------------------------------------|
*
*|### Schéma de câblage pour Arduino Nano V3: ---------------------------------------------------------------|
*
*| Composant                 | Broche du composant                           | Broche Arduino Nano V3        |
*|---------------------------|-----------------------------------------------|-------------------------------|
*| Bouton                    | Borne 1 / Borne 2                             | D10 / GND                     |
*| Chauffage (MOSFET)        | Grille (Gate)                                 | D3 (PWM)                      |
*| Ventilateur               | Commande (Transistor/MOSFET)                  | D2                            |
*| LED Rouge                 | Anode (+ via résistance) / Cathode (-)        | D8 / GND                      |
*| LED Verte                 | Anode (+ via résistance) / Cathode (-)        | D6 / GND                      |
*| Écran OLED (SSD1309/1306) | SDA                                           | A4                            |
*|            //             | SCL                                           | A5                            |
*|            //             | VCC                                           | 5V (ou 3.3V selon écran)      |
*|            //             | GND                                           | GND                           |
*| Buzzer (Optionnel)        | Pôle (+) / Pôle (-)                           | D4 / GND                      |
*| Sonde CTN (NTC 100k)      | Broche 1 / Broche 2 (via 47k vers +5V)        | A0 / GND                      |
*|---------------------------|-----------------------------------------------|-------------------------------|
*
*| ### Option Debug Moniteur Série: -------------------------------------------------------------------------|
*| Mettre à 1 pour activer les logs Série, ou à 0 pour désactiver et libérer de la mémoire (Flash & RAM)     |
*| #define ENABLE_SERIAL_DEBUG 0                                                                             |
*|-----------------------------------------------------------------------------------------------------------|
*/

#include "Globals.h"
#include "Hardware.h"
#include "Display.h"
#include "ScreenSaver.h"
#include "MenuLogic.h"

// --- Initialisation (setup) ---
void setup() {
#if ENABLE_SERIAL_DEBUG
  Serial.begin(115200);
#endif
  DEBUG_PRINTLN(F("\n=========================================="));
  DEBUG_PRINTLN(F(">>> ARDUINO SOUDEUSE : DEMARRAGE OK ! <<<"));
  DEBUG_PRINTLN(F("=========================================="));

  Wire.begin();

  DEBUG_PRINTLN(F("[ETAPE 1/3] Chargement de l'EEPROM..."));
  loadSettings(); // Charger ou initialiser les réglages EEPROM

  DEBUG_PRINTLN(F("[ETAPE 2/3] Initialisation et nettoyage ecran U8GLIB..."));
  // Nettoyage complet de la mémoire RAM de l'écran au démarrage
  u8g.firstPage();
  do { } while (u8g.nextPage());

  DEBUG_PRINTLN(F("[ETAPE 3/3] Affichage de l'ecran d'accueil..."));
  drawWelcomeScreen();
  delay(5000);
  
  applyDisplayInversion(settings.screenInverted);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(ntcPin, INPUT);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
  digitalWrite(fanPin, LOW);
  pinMode(switchPin, INPUT_PULLUP);
  randomSeed(analogRead(A6));

  workMode = modeMenu;
  lastActivityTime = millis();
  drawMenu();

  DEBUG_PRINTLN(F(">>> SUCCES : System prêt ! Navigation menu active."));
}

// --- Boucle Principale (loop) ---
void loop() {
  btn.tick();

  // --- 0. Gestion de la veille à 2 étapes (Screen Saver à 1 min, Extinction à 2 min) ---
  if (workMode == modeMenu) {
    unsigned long idleTime = millis() - lastActivityTime;

    // Étape 1 : Activer l'économiseur d'écran (1 à 2 minutes d'inactivité)
    if (!saverActive && !sleepActive && idleTime >= SAVER_TIMEOUT) {
      saverActive = true;
      initSparks();
      DEBUG_PRINTLN(F("\n[VEILLE] 1 min inactivité -> Lancement économiseur 'Pluie d'étincelles'"));
    }

    // Étape 2 : Extinction de l'écran après 2 minutes d'inactivité totale
    if (saverActive && !sleepActive && idleTime >= SLEEP_TIMEOUT) {
      sleepActive = true;
      DEBUG_PRINTLN(F("[VEILLE] 2 min inactivité -> Extinction totale de l'écran OLED"));
      u8g.firstPage();
      do { } while (u8g.nextPage());
      u8g.sleepOn();
    }
  } else {
    lastActivityTime = millis(); // Ne pas passer en veille pendant la soudure
  }

  // Animation de la pluie d'étincelles (rafraîchie toutes les 50ms quand l'économiseur est actif)
  if (saverActive && !sleepActive) {
    if (millis() - animTimer >= 50) {
      animTimer = millis();
      updateSparks();
      drawSparkScreenSaver();
    }
  }

  // Réveil du système lors d'un appui bouton (depuis l'économiseur OU le sommeil)
  if (saverActive || sleepActive) {
    if (btn.hasClicks()) {
      if (sleepActive) {
        u8g.sleepOff();
        applyDisplayInversion(settings.screenInverted);
      }
      saverActive = false;
      sleepActive = false;
      lastActivityTime = millis();
      DEBUG_PRINTLN(F("[VEILLE] Réveil du système par appui bouton."));
      drawMenu();
    }
    return; // Absorber le clic pour ne pas déclencher d'action menu
  }

  // --- Mode Erreur / Alerte Sécurité Thermique (Clic pour acquitter) ---
  if (workMode == errorMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("[SECURITE] Alarme acquittee par l'utilisateur."));
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 1. Mode Menu (Navigation & Réglages) ---
  if (workMode == modeMenu) {
    handleMenuNavigation();
  }
  // --- 2. Interruption de secours (Clic pendant la soudure -> Annuler) ---
  else if (workMode == heatingMode || workMode == warmingMode || workMode == coolingMode) {
    // Éviter l'annulation dans les 500 premières millisecondes après le lancement
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[ANNULATION] Soudure interrompue par l'utilisateur !"));
      analogWrite(mosfetPin, 0);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      digitalWrite(fanPin, HIGH);
      fanOn = true;
      workTimer = millis();
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }
  // Interruption du Refroidissement Forcé par un clic
  else if (workMode == forcedCoolMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[ANNULATION] Refroidissement force interrompu !"));
      fanOn = false;
      digitalWrite(fanPin, LOW);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 3. Machine à états du cycle de soudure & modes avec CTN ---
  if (workMode == heatingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    // Sécurité 1 : Sonde déconnectée / erreur
    if (isTempSensorError(currentTemp)) {
      switch2Emergency("ERREUR SONDE CTN", "Sonde deconnectee");
      return;
    }

    // Sécurité 2 : Alerte surchauffe critique
    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("ALERTE SURCHAUFFE", "Temperature > 295C");
      return;
    }

    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    // Condition de succès : Température cible atteinte !
    if (currentTemp >= (float)targetTemp) {
      DEBUG_PRINT(F("[CHAUFFE REUSSIE] Température atteinte : "));
      DEBUG_PRINT(currentTemp);
      DEBUG_PRINTLN(F("°C -> Bascule en Maintien"));
      switch2Warming();
    }
    // Sécurité 3 : Timeout dépassé (chauffe trop lente ou cartouche débranchée)
    else if (elapsed >= selectedHeatingPeriod) {
      switch2Emergency("SECURITE TIMEOUT", "Chauffe trop lente");
      return;
    }
    else {
      // Puissance adaptative : Boost si loin de la cible, Nominal à l'approche
      bool isBoost = (currentTemp < (float)(targetTemp - 25));
      if (isBoost) {
        analogWrite(mosfetPin, heatBoostPWM); // Boost de chauffe
      } else {
        analogWrite(mosfetPin, heatNominalPWM); // Chauffe nominale
      }

      int secLeft = (selectedHeatingPeriod - elapsed + 999) / 1000UL;
      if (secLeft != currentSecLeft || (millis() - iconAnimTimer >= 200)) {
        iconAnimTimer = millis();
        currentSecLeft = secLeft;
        int progress = constrain(map((int)currentTemp, 25, targetTemp, 0, 100), 0, 100);
        char statusBuf[24];
        if (isBoost) {
          snprintf(statusBuf, sizeof(statusBuf), "Chauffe Bst : %dC", (int)currentTemp);
        } else {
          snprintf(statusBuf, sizeof(statusBuf), "Chauffe : %dC", (int)currentTemp);
        }
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, isBoost, statusBuf);
      }
    }
  }

  if (workMode == warmingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    if (isTempSensorError(currentTemp)) {
      switch2Emergency("ERREUR SONDE CTN", "Sonde deconnectee");
      return;
    }
    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("ALERTE SURCHAUFFE", "Temperature > 295C");
      return;
    }

    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    if (elapsed >= ((unsigned long)settings.warmingPeriod * 1000UL)) {
      switch2Cooling();
    } else {
      // Régulation thermique précise pendant le maintien
      if (currentTemp < (float)(targetTemp - 3)) {
        analogWrite(mosfetPin, warmValue);
      } else {
        analogWrite(mosfetPin, 0);
      }

      blinkLED(redPin, warmingBlinkPeriod);
      int secLeft = (((unsigned long)settings.warmingPeriod * 1000UL) - elapsed + 999) / 1000UL;
      if (secLeft != currentSecLeft || (millis() - iconAnimTimer >= 200)) {
        iconAnimTimer = millis();
        currentSecLeft = secLeft;
        int progress = constrain((elapsed * 100UL) / ((unsigned long)settings.warmingPeriod * 1000UL), 0, 100);
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, warmingMode, false, "BOUGER DE <--|~|-->");
      }
    }
  }

  if (workMode == coolingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();
    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    // Fin de refroidissement : Température sous 45°C OU temporisation écoulée
    bool isCooled = (!isTempSensorError(currentTemp) && currentTemp <= (float)COOLDOWN_TARGET_TEMP);
    bool isTimeout = (elapsed >= ((unsigned long)settings.coolingPeriod * 1000UL));

    if (isCooled || isTimeout) {
      switch2Wait();
    } else {
      blinkLED(greenPin, coolingBlinkPeriod);
      int secLeft = (((unsigned long)settings.coolingPeriod * 1000UL) - elapsed + 999) / 1000UL;
      if (secLeft < 0) secLeft = 0;

      if (secLeft != currentSecLeft || (millis() - iconAnimTimer >= 200)) {
        iconAnimTimer = millis();
        currentSecLeft = secLeft;

        int progress = 0;
        if (!isTempSensorError(currentTemp)) {
          progress = constrain(map((int)currentTemp, targetTemp, COOLDOWN_TARGET_TEMP, 0, 100), 0, 100);
        } else {
          progress = constrain((elapsed * 100UL) / ((unsigned long)settings.coolingPeriod * 1000UL), 0, 100);
        }

        char coolMsg[24];
        if (!isTempSensorError(currentTemp)) {
          if ((millis() / 1500) % 2 == 0) {
            snprintf(coolMsg, sizeof(coolMsg), "Refroid. : %dC", (int)currentTemp);
          } else {
            snprintf(coolMsg, sizeof(coolMsg), "BOUGER <--|~|-->");
          }
        } else {
          snprintf(coolMsg, sizeof(coolMsg), "Refroidissement");
        }

        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, coolingMode, false, coolMsg);
      }
    }
  }

  if (workMode == forcedCoolMode) {
    unsigned long elapsed = millis() - workTimer;
    if (elapsed >= 30000UL) {
      fanOn = false;
      digitalWrite(fanPin, LOW);
      workMode = modeMenu;
      drawMenu();
    } else {
      int secLeft = (30000UL - elapsed + 999) / 1000UL;
      if (secLeft != currentSecLeft || (millis() - iconAnimTimer >= 150)) {
        iconAnimTimer = millis();
        currentSecLeft = secLeft;
        int progress = constrain((elapsed * 100UL) / 30000UL, 0, 100);
        drawForcedCoolScreen(currentSecLeft, progress);
      }
    }
  }

  if (workMode == waitMode) {
    // Après 5 secondes d'affichage "Soudure terminée", retour au menu principal
    if (millis() - workTimer >= 5000UL) {
      workMode = modeMenu;
      drawMenu();
    }
  }

  // --- 4. Extinction intelligente du ventilateur ---
  if (workMode == modeMenu || workMode == waitMode) {
    if (fanOn) {
      float currentTemp = readTemperature();
      bool isCool = (!isTempSensorError(currentTemp) && currentTemp <= (float)COOLDOWN_TARGET_TEMP);
      bool isTimeout = (millis() - workTimer >= ((unsigned long)settings.fanPeriod * 1000UL));

      if (isCool || isTimeout) {
        fanOn = false;
        digitalWrite(fanPin, LOW);
        DEBUG_PRINTLN(F("Fan off (Bloc refroidi)"));
      }
    }
  }
}
