/*
*|-----------------------------------------------------------------------------------------------------------|
*| Firmware        : Automated 3D Filament Welder (PWM) - Modular C++ Version                                |
*| Original Author : ptica (version v3.0)                                                                    |
*| Enhanced Author : ZelTroN2k3 (enhanced version v3.8 Alpha)                                                |
*| Hardware        : Arduino Nano V3 (ATmega328P) + OLED I2C 128x64 (SSD1309 / SSD1306 / SH1106)             |
*|-----------------------------------------------------------------------------------------------------------|
*
*|### Wiring Pinout for Arduino Nano V3: --------------------------------------------------------------------|
*
*| Component                 | Component Pin                                 | Arduino Nano V3 Pin           |
*|---------------------------|-----------------------------------------------|-------------------------------|
*| Push Button               | Terminal 1 / Terminal 2                       | D10 / GND                     |
*| Heater (MOSFET)           | Gate                                          | D3 (PWM)                      |
*| Fan                       | Control (Transistor/MOSFET)                   | D2                            |
*| Red LED                   | Anode (+ via resistor) / Cathode (-)          | D8 / GND                      |
*| Green LED                 | Anode (+ via resistor) / Cathode (-)          | D6 / GND                      |
*| OLED Display (SSD1306)    | SDA                                           | A4                            |
*|            //             | SCL                                           | A5                            |
*|            //             | VCC                                           | 5V (or 3.3V)                  |
*|            //             | GND                                           | GND                           |
*| Buzzer (Optional)         | (+) / (-)                                     | D4 / GND                      |
*| NTC Sensor (100k B3950)   | Terminal 1 / Terminal 2 (via 47k to +5V)      | A0 / GND                      |
*|---------------------------|-----------------------------------------------|-------------------------------|
*/

#include "Globals.h"
#include "Hardware.h"
#include "Display.h"
#include "ScreenSaver.h"
#include "MenuLogic.h"

// --- Setup ---
void setup() {
#if ENABLE_SERIAL_DEBUG
  Serial.begin(115200);
#endif
  DEBUG_PRINTLN(F("\n=========================================="));
  DEBUG_PRINTLN(F(">>> ARDUINO WELDER : STARTUP READY ! <<<"));
  DEBUG_PRINTLN(F("=========================================="));

  Wire.begin();

  DEBUG_PRINTLN(F("[STEP 1/3] Loading EEPROM settings..."));
  loadSettings();

  DEBUG_PRINTLN(F("[STEP 2/3] Initializing and clearing U8GLIB display..."));
  u8g.firstPage();
  do { } while (u8g.nextPage());

  DEBUG_PRINTLN(F("[STEP 3/3] Drawing Welcome Screen..."));
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

  DEBUG_PRINTLN(F(">>> SUCCESS : System ready! Main menu active."));
}

// --- Main Loop ---
void loop() {
  btn.tick();

  // --- 0. Screen Sleep Management ---
  if (workMode == modeMenu) {
    unsigned long idleTime = millis() - lastActivityTime;

    if (!saverActive && !sleepActive && idleTime >= SAVER_TIMEOUT) {
      saverActive = true;
      initSparks();
      DEBUG_PRINTLN(F("\n[SLEEP] 1 min idle -> Spark screensaver started"));
    }

    if (saverActive && !sleepActive && idleTime >= SLEEP_TIMEOUT) {
      sleepActive = true;
      DEBUG_PRINTLN(F("[SLEEP] 2 min idle -> Full OLED sleep"));
      u8g.firstPage();
      do { } while (u8g.nextPage());
      u8g.sleepOn();
    }
  } else {
    lastActivityTime = millis();
  }

  // Screensaver animation
  if (saverActive && !sleepActive) {
    if (millis() - animTimer >= 50) {
      animTimer = millis();
      updateSparks();
      drawSparkScreenSaver();
    }
  }

  // Wakeup from sleep
  if (saverActive || sleepActive) {
    if (btn.hasClicks()) {
      if (sleepActive) {
        u8g.sleepOff();
        applyDisplayInversion(settings.screenInverted);
      }
      saverActive = false;
      sleepActive = false;
      lastActivityTime = millis();
      DEBUG_PRINTLN(F("[SLEEP] System woken up by button click."));
      drawMenu();
    }
    return;
  }

  // --- Error / Thermal Safety Alarm Mode ---
  if (workMode == errorMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("[SAFETY] Alarm acknowledged by user."));
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 1. Menu Mode ---
  if (workMode == modeMenu) {
    handleMenuNavigation();
  }
  // --- 2. Emergency Interruption ---
  else if (workMode == heatingMode || workMode == warmingMode || workMode == coolingMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[CANCEL] Welding aborted by user !"));
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
  // Cancel Forced Cooling
  else if (workMode == forcedCoolMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[CANCEL] Forced cooling aborted !"));
      fanOn = false;
      digitalWrite(fanPin, LOW);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 3. Welding State Machine with NTC Regulation ---
  if (workMode == heatingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    // Safety 1: Sensor Error
    if (isTempSensorError(currentTemp)) {
      switch2Emergency("NTC SENSOR ERROR", "Sensor disconnected");
      return;
    }

    // Safety 2: Critical Overheat
    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("OVERHEAT ALERT", "Temperature > 295C");
      return;
    }

    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    // Target temperature reached
    if (currentTemp >= (float)targetTemp) {
      DEBUG_PRINT(F("[HEATING SUCCESS] Target reached : "));
      DEBUG_PRINT(currentTemp);
      DEBUG_PRINTLN(F("°C -> Switching to Holding"));
      switch2Warming();
    }
    // Safety 3: Timeout exceeded
    else if (elapsed >= selectedHeatingPeriod) {
      switch2Emergency("TIMEOUT SAFETY", "Heating too slow");
      return;
    }
    else {
      bool isBoost = (currentTemp < (float)(targetTemp - 25));
      if (isBoost) {
        analogWrite(mosfetPin, heatBoostPWM);
      } else {
        analogWrite(mosfetPin, heatNominalPWM);
      }

      int secLeft = (selectedHeatingPeriod - elapsed + 999) / 1000UL;
      if (secLeft != currentSecLeft || (millis() - iconAnimTimer >= 200)) {
        iconAnimTimer = millis();
        currentSecLeft = secLeft;
        int progress = constrain(map((int)currentTemp, 25, targetTemp, 0, 100), 0, 100);
        char statusBuf[24];
        if (isBoost) {
          snprintf(statusBuf, sizeof(statusBuf), "Boost Heat : %dC", (int)currentTemp);
        } else {
          snprintf(statusBuf, sizeof(statusBuf), "Heating : %dC", (int)currentTemp);
        }
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, isBoost, statusBuf);
      }
    }
  }

  if (workMode == warmingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    if (isTempSensorError(currentTemp)) {
      switch2Emergency("NTC SENSOR ERROR", "Sensor disconnected");
      return;
    }
    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("OVERHEAT ALERT", "Temperature > 295C");
      return;
    }

    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    if (elapsed >= ((unsigned long)settings.warmingPeriod * 1000UL)) {
      switch2Cooling();
    } else {
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
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, warmingMode, false, "MOVE <--|~|-->");
      }
    }
  }

  if (workMode == coolingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();
    int targetTemp = getTargetTemperature(selectedMaterialIdx);

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
            snprintf(coolMsg, sizeof(coolMsg), "Cooling : %dC", (int)currentTemp);
          } else {
            snprintf(coolMsg, sizeof(coolMsg), "MOVE <--|~|-->");
          }
        } else {
          snprintf(coolMsg, sizeof(coolMsg), "Cooling...");
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
    if (millis() - workTimer >= 5000UL) {
      workMode = modeMenu;
      drawMenu();
    }
  }

  // --- 4. Intelligent Fan Shutdown ---
  if (workMode == modeMenu || workMode == waitMode) {
    if (fanOn) {
      float currentTemp = readTemperature();
      bool isCool = (!isTempSensorError(currentTemp) && currentTemp <= (float)COOLDOWN_TARGET_TEMP);
      bool isTimeout = (millis() - workTimer >= ((unsigned long)settings.fanPeriod * 1000UL));

      if (isCool || isTimeout) {
        fanOn = false;
        digitalWrite(fanPin, LOW);
        DEBUG_PRINTLN(F("Fan off (Block cooled down)"));
      }
    }
  }
}
