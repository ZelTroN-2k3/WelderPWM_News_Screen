#include "Display.h"

// --- Экран приветствия с U8glib ---
void drawWelcomeScreen() {
  u8g.firstPage();
  do {
    // Рамка по 4 углам экрана (стиль HUD)
    u8g.drawLine(2, 2, 14, 2);
    u8g.drawLine(2, 2, 2, 14);

    u8g.drawLine(125, 2, 113, 2);
    u8g.drawLine(125, 2, 125, 14);

    u8g.drawLine(2, 61, 14, 61);
    u8g.drawLine(2, 61, 2, 49);

    u8g.drawLine(125, 61, 113, 61);
    u8g.drawLine(125, 61, 125, 49);

    // Заголовок
    u8g.setFont(u8g_font_7x14B);
    int titleX = (128 - u8g.getStrWidth("SOUDEUSE")) / 2;
    u8g.drawStrP(titleX, 22, PSTR("SOUDEUSE"));

    // Авторы
    u8g.setFont(u8g_font_6x10);
    int authorX = (128 - u8g.getStrWidth("ptica & ZelTroN2k3")) / 2;
    u8g.drawStrP(authorX, 38, PSTR("ptica & ZelTroN2k3"));

    // Версия
    int verX = (128 - u8g.getStrWidth(FIRMWARE_VERSION)) / 2;
    u8g.drawStrP(verX, 52, PSTR(FIRMWARE_VERSION));
  } while (u8g.nextPage());
}

// --- Информационный экран ---
void drawMessageScreen(const char* line1, const char* line2) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(12, 28, line1);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(10, 48, line2);
  } while (u8g.nextPage());
}

// --- Анимация мини-иконок фазы (вверху по центру x=64, y=8) ---
void drawPhaseIcon(int phaseMode, int frame, bool isBoost) {
  int cx = 64;
  int cy = 8;

  // ⚡ / 🔥 Фаза 1 : Нагрев
  if (phaseMode == heatingMode) {
    if (isBoost) {
      u8g.drawLine(cx + 1, cy - 6, cx - 2, cy - 1);
      u8g.drawLine(cx - 2, cy - 1, cx + 2, cy - 1);
      u8g.drawLine(cx + 2, cy - 1, cx - 1, cy + 5);
      if (frame % 2 == 1) {
        u8g.drawPixel(cx + 3, cy - 4);
        u8g.drawPixel(cx - 3, cy + 2);
      }
    } else {
      u8g.drawFrame(cx - 2, cy - 6, 5, 8);
      u8g.drawDisc(cx, cy + 3, 2);
      int h = (frame % 4) + 1;
      u8g.drawLine(cx, cy + 2, cx, cy + 2 - h);
    }
  }
  // ↔️ Фаза 2 : Удержание
  else if (phaseMode == warmingMode) {
    int shift = frame % 4;
    if (shift == 3) shift = 1;
    u8g.drawLine(cx - 7, cy, cx - 4, cy - 3);
    u8g.drawLine(cx - 7, cy, cx - 4, cy + 3);
    u8g.drawBox(cx - 2 + (shift * 2 - 2), cy - 3, 3, 7);
    u8g.drawLine(cx + 7, cy, cx + 4, cy - 3);
    u8g.drawLine(cx + 7, cy, cx + 4, cy + 3);
  }
  // 🌀 Фаза 3 & Принудительное охлаждение : Вентилятор
  else if (phaseMode == coolingMode || phaseMode == forcedCoolMode) {
    u8g.drawPixel(cx, cy);
    int f = frame % 4;
    if (f == 0 || f == 2) {
      u8g.drawLine(cx - 5, cy, cx + 5, cy);
      u8g.drawLine(cx, cy - 5, cx, cy + 5);
      u8g.drawPixel(cx + 4, cy - 1);
      u8g.drawPixel(cx - 4, cy + 1);
      u8g.drawPixel(cx + 1, cy + 4);
      u8g.drawPixel(cx - 1, cy - 4);
    } else {
      u8g.drawLine(cx - 4, cy - 4, cx + 4, cy + 4);
      u8g.drawLine(cx - 4, cy + 4, cx + 4, cy - 4);
      u8g.drawPixel(cx + 3, cy - 4);
      u8g.drawPixel(cx - 3, cy + 4);
      u8g.drawPixel(cx + 4, cy + 3);
      u8g.drawPixel(cx - 4, cy - 3);
    }
  }
}

// --- Графический экран сварки с таймером и прогресс-баром ---
void drawMaterialScreen(int modeIdx, int secondsLeft, int progressPercent, int phaseMode, bool isBoost, const char* status1) {
  u8g.firstPage();
  do {
    // --- 1. Верхняя строка ---
    u8g.setFont(u8g_font_7x14B);
    if (modeIdx == 0) u8g.drawStrP(2, 14, PSTR("Mode 1"));
    else if (modeIdx == 1) u8g.drawStrP(2, 14, PSTR("Mode 2"));
    else if (modeIdx == 2) u8g.drawStrP(2, 14, PSTR("Mode 3"));
    else u8g.drawStrP(2, 14, PSTR("Mode 4"));

    u8g.drawLine(48, 0, 48, 17);
    drawPhaseIcon(phaseMode, (millis() / 200) % 4, isBoost);
    u8g.drawLine(80, 0, 80, 17);

    // Таймер обратного отсчета
    if (secondsLeft >= 0) {
      char timeBuf[8];
      sprintf(timeBuf, "%us", secondsLeft);
      int xPos = 126 - u8g.getStrWidth(timeBuf);
      u8g.drawStr(xPos, 14, timeBuf);
    }

    u8g.drawLine(0, 17, 127, 17);

    // --- 2. Центральная зона ---
    u8g.setFont(u8g_font_fub17r);
    if (modeIdx == 0) u8g.drawStrP(2, 38, PSTR("PET"));
    else if (modeIdx == 1) u8g.drawStrP(2, 38, PSTR("PETG"));
    else if (modeIdx == 2) u8g.drawStrP(2, 38, PSTR("PLA"));
    else u8g.drawStrP(2, 38, PSTR("CUST"));

    // Целевая температура (по правому краю)
    u8g.setFont(u8g_font_6x10);
    int tempVal = getTargetTemperature(modeIdx);

    if (tempVal > 0) {
      char tBuf[8];
      sprintf(tBuf, "%u", tempVal);
      int strW = u8g.getStrWidth(tBuf);
      int startX = 124 - (strW + 8);
      u8g.drawStr(startX, 35, tBuf);
      int degX = startX + strW + 2;
      u8g.drawCircle(degX, 29, 1);
      u8g.drawStrP(degX + 3, 35, PSTR("C"));
    }

    u8g.drawLine(0, 42, 127, 42);

    // --- 3. Нижняя зона ---
    u8g.setFont(u8g_font_6x10);
    if (status1) u8g.drawStr(2, 52, status1);

    // Прогресс-бар
    u8g.drawFrame(2, 55, 124, 7);
    int p = constrain(progressPercent, 0, 100);
    if (p > 0) {
      int fillW = map(p, 0, 100, 0, 120);
      if (fillW > 0) {
        u8g.drawBox(4, 57, fillW, 3);
      }
    }
  } while (u8g.nextPage());
}

// --- Экран принудительного охлаждения (30s) ---
void drawForcedCoolScreen(int secLeft, int progress) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStrP(2, 12, PSTR("VENTILATSIYA (30s)"));

    char timeBuf[8];
    sprintf(timeBuf, "%us", secLeft);
    int xPos = 126 - u8g.getStrWidth(timeBuf);
    u8g.drawStr(xPos, 12, timeBuf);

    int cx = 64;
    int cy = 26;
    int f = (millis() / 150) % 4;
    u8g.drawCircle(cx, cy, 10);
    u8g.drawDisc(cx, cy, 2);
    if (f == 0 || f == 2) {
      u8g.drawLine(cx - 9, cy, cx + 9, cy);
      u8g.drawLine(cx, cy - 9, cx, cy + 9);
      u8g.drawPixel(cx + 7, cy - 2);
      u8g.drawPixel(cx - 7, cy + 2);
      u8g.drawPixel(cx + 1, cy + 4);
      u8g.drawPixel(cx - 1, cy - 4);
    } else {
      u8g.drawLine(cx - 7, cy - 7, cx + 7, cy + 7);
      u8g.drawLine(cx - 7, cy + 7, cx + 7, cy - 7);
      u8g.drawPixel(cx + 6, cy - 6);
      u8g.drawPixel(cx - 6, cy + 6);
      u8g.drawPixel(cx + 6, cy + 6);
      u8g.drawPixel(cx - 6, cy - 6);
    }

    u8g.drawStrP(12, 47, PSTR("Klik dlya otmeny"));

    u8g.drawFrame(2, 53, 124, 8);
    int p = constrain(progress, 0, 100);
    if (p > 0) {
      int fillW = map(p, 0, 100, 0, 120);
      if (fillW > 0) u8g.drawBox(4, 55, fillW, 4);
    }
  } while (u8g.nextPage());
}

// --- Экран завершения сварки ---
void drawWaitScreen() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStrP(12, 20, PSTR("Svarka OK !"));

    u8g.setFont(u8g_font_6x10);
    u8g.drawStrP(12, 36, PSTR("Tsikl zavershen."));

    char countBuf[24];
    sprintf(countBuf, "Vsego: %u svarok", settings.weldCounter);
    u8g.drawStr(12, 52, countBuf);
  } while (u8g.nextPage());
}

// --- Экран аварийной тепловой защиты ---
void drawEmergencyScreen(const char* title, const char* reason) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(6, 18, title);

    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(4, 36, reason);
    u8g.drawStrP(4, 52, PSTR("Klik dlya sbrosa"));
  } while (u8g.nextPage());
}

// --- Отрисовка динамического меню ---
void drawMenu() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    if (currentMenuPage == PAGE_MAIN) {
      u8g.drawStrP(5, 10, PSTR("--- PAYALNIK PWM ---"));

      for (int i = 0; i < MAIN_MENU_COUNT; i++) {
        int y = 22 + (i * 10);
        if (i == menuCursor) {
          u8g.drawStr(2, y, ">");
        }
        u8g.drawStrP(12, y, (u8g_pgm_uint8_t*)pgm_read_word(&(mainMenuItems[i])));

        // Целевая температура режима
        if (i < 4) {
          int target = getTargetTemperature(i);
          char buf[8];
          sprintf(buf, "%uC", target);
          u8g.drawStr(100, y, buf);
        }
      }
    } 
    else if (currentMenuPage == PAGE_SETTINGS) {
      u8g.drawStrP(5, 10, PSTR("----* NASTROYKI *----"));

      int startItem = 0;
      if (menuCursor > 4) {
        startItem = menuCursor - 4;
      }

      for (int i = 0; i < 5; i++) {
        int itemIdx = startItem + i;
        if (itemIdx >= SETTINGS_MENU_COUNT) break;

        int y = 22 + (i * 10);
        if (itemIdx == menuCursor) {
          if (isEditing) {
            u8g.drawStr(2, y, "*");
          } else {
            u8g.drawStr(2, y, ">");
          }
        }

        u8g.drawStrP(12, y, (u8g_pgm_uint8_t*)pgm_read_word(&(settingsMenuItems[itemIdx])));

        uint16_t* valPtr = getSettingsValuePtr(itemIdx);
        if (valPtr != NULL) {
          if (itemIdx == 9) {
            if (*valPtr == 1) u8g.drawStrP(106, y, PSTR("DA"));
            else u8g.drawStrP(106, y, PSTR("NET"));
          } else if (itemIdx == 7) {
            char buf[8];
            sprintf(buf, "%uC", *valPtr);
            u8g.drawStr(100, y, buf);
          } else {
            char buf[8];
            sprintf(buf, "%u", *valPtr);
            u8g.drawStr(105, y, buf);
          }
        }
      }
    }
  } while (u8g.nextPage());
}
