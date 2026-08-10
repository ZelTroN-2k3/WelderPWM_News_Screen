/*
*|-----------------------------------------------------------------------------------------------------------|
*| Прошивка        : Автоматизированный паяльник филамента 3D (ШИМ) - Модульная C++ версия                   |
*| Автор оригинала : ptica (версия v3.0)                                                                     |
*| Автор изменений : ZelTroN2k3 (версия v3.8 Alpha)                                                          |
*| Оборудование    : Arduino Nano V3 (ATmega328P) + OLED I2C 128x64 (SSD1309 / SSD1306 / SH1106)             |
*|-----------------------------------------------------------------------------------------------------------|
*
*|### Схема подключения для Arduino Nano V3: ---------------------------------------------------------------|
*
*| Компонент                 | Контакт компонента                            | Пин Arduino Nano V3           |
*|---------------------------|-----------------------------------------------|-------------------------------|
*| Кнопка                    | Контакт 1 / Контакт 2                         | D10 / GND                     |
*| Нагреватель (MOSFET)      | Затвор (Gate)                                 | D3 (PWM)                      |
*| Вентилятор                | Управление (Транзистор/MOSFET)                | D2                            |
*| Красный светодиод         | Анод (+ через резистор) / Катод (-)           | D8 / GND                      |
*| Зеленый светодиод         | Анод (+ через резистор) / Катод (-)           | D6 / GND                      |
*| OLED дисплей (SSD1306)    | SDA                                           | A4                            |
*|            //             | SCL                                           | A5                            |
*|            //             | VCC                                           | 5V (или 3.3V)                 |
*|            //             | GND                                           | GND                           |
*| Зуммер (Опционально)      | (+) / (-)                                     | D4 / GND                      |
*| Датчик NTC (100k B3950)   | Контакт 1 / Контакт 2 (через 47k к +5V)       | A0 / GND                      |
*|---------------------------|-----------------------------------------------|-------------------------------|
*/

#include "Globals.h"
#include "Hardware.h"
#include "Display.h"
#include "ScreenSaver.h"
#include "MenuLogic.h"

// --- Инициализация (setup) ---
void setup() {
#if ENABLE_SERIAL_DEBUG
  Serial.begin(115200);
#endif
  DEBUG_PRINTLN(F("\n=========================================="));
  DEBUG_PRINTLN(F(">>> ARDUINO SOUDEUSE : ZAPUSK OK ! <<<"));
  DEBUG_PRINTLN(F("=========================================="));

  Wire.begin();

  DEBUG_PRINTLN(F("[SHAG 1/3] Zagruzka EEPROM..."));
  loadSettings();

  DEBUG_PRINTLN(F("[SHAG 2/3] Ochistka ekrana U8GLIB..."));
  u8g.firstPage();
  do { } while (u8g.nextPage());

  DEBUG_PRINTLN(F("[SHAG 3/3] Otrisovka ekrana privetstviya..."));
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

  DEBUG_PRINTLN(F(">>> USPEKH : Sistema gotova k rabote."));
}

// --- Главный цикл (loop) ---
void loop() {
  btn.tick();

  // --- 0. Спящий режим ---
  if (workMode == modeMenu) {
    unsigned long idleTime = millis() - lastActivityTime;

    if (!saverActive && !sleepActive && idleTime >= SAVER_TIMEOUT) {
      saverActive = true;
      initSparks();
      DEBUG_PRINTLN(F("\n[SON] 1 min prostoy -> Zapusk zastavki"));
    }

    if (saverActive && !sleepActive && idleTime >= SLEEP_TIMEOUT) {
      sleepActive = true;
      DEBUG_PRINTLN(F("[SON] 2 min prostoy -> Otklyuchenie ekrana"));
      u8g.firstPage();
      do { } while (u8g.nextPage());
      u8g.sleepOn();
    }
  } else {
    lastActivityTime = millis();
  }

  // Анимация заставки
  if (saverActive && !sleepActive) {
    if (millis() - animTimer >= 50) {
      animTimer = millis();
      updateSparks();
      drawSparkScreenSaver();
    }
  }

  // Пробуждение
  if (saverActive || sleepActive) {
    if (btn.hasClicks()) {
      if (sleepActive) {
        u8g.sleepOff();
        applyDisplayInversion(settings.screenInverted);
      }
      saverActive = false;
      sleepActive = false;
      lastActivityTime = millis();
      DEBUG_PRINTLN(F("[SON] Probuzhdenie po nazhatiyu knopki."));
      drawMenu();
    }
    return;
  }

  // --- Режим аварийного предупреждения ---
  if (workMode == errorMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("[AVARIYA] Sbros avarii polzovatelem."));
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 1. Режим Меню ---
  if (workMode == modeMenu) {
    handleMenuNavigation();
  }
  // --- 2. Аварийная остановка сварки ---
  else if (workMode == heatingMode || workMode == warmingMode || workMode == coolingMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[OTMENA] Svarka prervana polzovatelem !"));
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
  // Отмена принудительного охлаждения
  else if (workMode == forcedCoolMode) {
    if (btn.hasClicks() && (millis() - workTimer > 500)) {
      DEBUG_PRINTLN(F("\n[OTMENA] Okhlazhdenie prervano !"));
      fanOn = false;
      digitalWrite(fanPin, LOW);
      workMode = modeMenu;
      drawMenu();
      return;
    }
  }

  // --- 3. Автомат состояний сварки с регулированием NTC ---
  if (workMode == heatingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    if (isTempSensorError(currentTemp)) {
      switch2Emergency("OSHIBKA DATCHIKA", "Datchik otkluchen");
      return;
    }

    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("PEREGREV BLOKA !", "Temperatura > 295C");
      return;
    }

    int targetTemp = getTargetTemperature(selectedMaterialIdx);

    if (currentTemp >= (float)targetTemp) {
      DEBUG_PRINT(F("[NAGREV OK] Temperatura dostignuta : "));
      DEBUG_PRINT(currentTemp);
      DEBUG_PRINTLN(F("°C -> Perekhod k Uderzhaniyu"));
      switch2Warming();
    }
    else if (elapsed >= selectedHeatingPeriod) {
      switch2Emergency("TAYMAUT NAGREVA", "Nagrev slishkom medlenny");
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
          snprintf(statusBuf, sizeof(statusBuf), "Nagrev Bst : %dC", (int)currentTemp);
        } else {
          snprintf(statusBuf, sizeof(statusBuf), "Nagrev : %dC", (int)currentTemp);
        }
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, heatingMode, isBoost, statusBuf);
      }
    }
  }

  if (workMode == warmingMode) {
    unsigned long elapsed = millis() - workTimer;
    float currentTemp = readTemperature();

    if (isTempSensorError(currentTemp)) {
      switch2Emergency("OSHIBKA DATCHIKA", "Datchik otkluchen");
      return;
    }
    if (currentTemp >= (float)NTC_MAX_SAFE_TEMP) {
      switch2Emergency("PEREGREV BLOKA !", "Temperatura > 295C");
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
        drawMaterialScreen(selectedMaterialIdx, currentSecLeft, progress, warmingMode, false, "DVIZHENIE <--|~|-->");
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
            snprintf(coolMsg, sizeof(coolMsg), "Okhlazhd. : %dC", (int)currentTemp);
          } else {
            snprintf(coolMsg, sizeof(coolMsg), "DVIZHENIE <--|~|-->");
          }
        } else {
          snprintf(coolMsg, sizeof(coolMsg), "Okhlazhdenie...");
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

  // --- 4. Интеллектуальное отключение вентилятора ---
  if (workMode == modeMenu || workMode == waitMode) {
    if (fanOn) {
      float currentTemp = readTemperature();
      bool isCool = (!isTempSensorError(currentTemp) && currentTemp <= (float)COOLDOWN_TARGET_TEMP);
      bool isTimeout = (millis() - workTimer >= ((unsigned long)settings.fanPeriod * 1000UL));

      if (isCool || isTimeout) {
        fanOn = false;
        digitalWrite(fanPin, LOW);
        DEBUG_PRINTLN(F("Fan off (Blok ostyl)"));
      }
    }
  }
}
