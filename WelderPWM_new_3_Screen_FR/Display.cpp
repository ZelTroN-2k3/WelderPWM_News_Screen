#include "Display.h"

// --- Écran d'accueil avec U8glib ---
void drawWelcomeScreen() {
  u8g.firstPage();
  do {
    // Cadre graphique aux 4 coins de l'écran (style HUD / Viseur technologique)
    // Coin haut-gauche ┌
    u8g.drawLine(2, 2, 14, 2);
    u8g.drawLine(2, 2, 2, 14);

    // Coin haut-droit ┐
    u8g.drawLine(125, 2, 113, 2);
    u8g.drawLine(125, 2, 125, 14);

    // Coin bas-gauche └
    u8g.drawLine(2, 61, 14, 61);
    u8g.drawLine(2, 61, 2, 49);

    // Coin bas-droit ┘
    u8g.drawLine(125, 61, 113, 61);
    u8g.drawLine(125, 61, 125, 49);

    // Titre centré
    u8g.setFont(u8g_font_7x14B);
    int titleX = (128 - u8g.getStrWidth("SOUDEUSE")) / 2;
    u8g.drawStrP(titleX, 22, PSTR("SOUDEUSE"));

    // Auteurs (Auteur initial ptica & Auteur évolué ZelTroN2k3)
    u8g.setFont(u8g_font_6x10);
    int authorX = (128 - u8g.getStrWidth("ptica & ZelTroN2k3")) / 2;
    u8g.drawStrP(authorX, 38, PSTR("ptica & ZelTroN2k3"));

    // Version centrée
    int verX = (128 - u8g.getStrWidth(FIRMWARE_VERSION)) / 2;
    u8g.drawStrP(verX, 52, PSTR(FIRMWARE_VERSION));
  } while (u8g.nextPage());
}

// --- Écran de notification éphémère ---
void drawMessageScreen(const char* line1, const char* line2) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(12, 28, line1);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(10, 48, line2);
  } while (u8g.nextPage());
}

// --- Dessin des icônes animées de chaque phase (en haut au centre x=64, y=8) ---
void drawPhaseIcon(int phaseMode, int frame, bool isBoost) {
  int cx = 64; // Position X centrale
  int cy = 8;  // Position Y centrale

  // ⚡ / 🔥 Phase 1 : Chauffe
  if (phaseMode == heatingMode) {
    if (isBoost) {
      // ⚡ Éclair animé de Boost
      u8g.drawLine(cx + 1, cy - 6, cx - 2, cy - 1);
      u8g.drawLine(cx - 2, cy - 1, cx + 2, cy - 1);
      u8g.drawLine(cx + 2, cy - 1, cx - 1, cy + 5);
      if (frame % 2 == 1) {
        u8g.drawPixel(cx + 3, cy - 4);
        u8g.drawPixel(cx - 3, cy + 2);
      }
    } else {
      // Thermomètre avec niveau montant
      u8g.drawFrame(cx - 2, cy - 6, 5, 8);
      u8g.drawDisc(cx, cy + 3, 2);
      int h = (frame % 4) + 1;
      u8g.drawLine(cx, cy + 2, cx, cy + 2 - h);
    }
  }
  // ↔️ Phase 2 : Maintien (Flèches et curseur va-et-vient)
  else if (phaseMode == warmingMode) {
    int shift = frame % 4;
    if (shift == 3) shift = 1; // Oscillation : 0, 1, 2, 1
    u8g.drawLine(cx - 7, cy, cx - 4, cy - 3);
    u8g.drawLine(cx - 7, cy, cx - 4, cy + 3);
    u8g.drawBox(cx - 2 + (shift * 2 - 2), cy - 3, 3, 7);
    u8g.drawLine(cx + 7, cy, cx + 4, cy - 3);
    u8g.drawLine(cx + 7, cy, cx + 4, cy + 3);
  }
  // 🌀 Phase 3 & Refroidissement Forcé : Hélice de ventilateur tournante
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

// --- Affichage graphique du matériau, compte à rebours, icône et barre de progression (Design Cloisonné) ---
void drawMaterialScreen(int modeIdx, int secondsLeft, int progressPercent, int phaseMode, bool isBoost, const char* status1) {
  u8g.firstPage();
  do {
    // --- 1. BANDEAU SUPÉRIEUR (Header cloisonné) ---
    u8g.setFont(u8g_font_7x14B);
    if (modeIdx == 0) u8g.drawStrP(2, 14, PSTR("Mode 1"));
    else if (modeIdx == 1) u8g.drawStrP(2, 14, PSTR("Mode 2"));
    else if (modeIdx == 2) u8g.drawStrP(2, 14, PSTR("Mode 3"));
    else u8g.drawStrP(2, 14, PSTR("Mode 4"));

    // Séparateur vertical gauche
    u8g.drawLine(48, 0, 48, 17);

    // Mini-icône animée de phase (centre)
    drawPhaseIcon(phaseMode, (millis() / 200) % 4, isBoost);

    // Séparateur vertical droit
    u8g.drawLine(80, 0, 80, 17);

    // Compte à rebours en secondes (droite)
    if (secondsLeft >= 0) {
      char timeBuf[8];
      sprintf(timeBuf, "%us", secondsLeft);
      int xPos = 126 - u8g.getStrWidth(timeBuf);
      u8g.drawStr(xPos, 14, timeBuf);
    }

    // Ligne horizontale sous le bandeau supérieur
    u8g.drawLine(0, 17, 127, 17);

    // --- 2. ZONE CENTRALE (Matériau à gauche & Température à droite) ---
    // Nom du matériau (gauche en gras FreeUniversal Bold 17)
    u8g.setFont(u8g_font_fub17r);
    if (modeIdx == 0) u8g.drawStrP(2, 38, PSTR("PET"));
    else if (modeIdx == 1) u8g.drawStrP(2, 38, PSTR("PETG"));
    else if (modeIdx == 2) u8g.drawStrP(2, 38, PSTR("PLA"));
    else u8g.drawStrP(2, 38, PSTR("CUST"));

    // Température cible (alignée à droite avec vrai symbole ° : ex "280°C")
    u8g.setFont(u8g_font_6x10);
    int tempVal = getTargetTemperature(modeIdx);

    if (tempVal > 0) {
      char tBuf[8];
      sprintf(tBuf, "%u", tempVal);
      int strW = u8g.getStrWidth(tBuf);
      int startX = 124 - (strW + 8); // Aligné à droite (finit à x=126)
      u8g.drawStr(startX, 35, tBuf);
      int degX = startX + strW + 2;
      u8g.drawCircle(degX, 29, 1); // Symbole degré °
      u8g.drawStrP(degX + 3, 35, PSTR("C"));
    }

    // Ligne horizontale sous la zone centrale
    u8g.drawLine(0, 42, 127, 42);

    // --- 3. ZONE INFÉRIEURE (Conseil d'action & Barre de progression) ---
    u8g.setFont(u8g_font_6x10);
    if (status1) u8g.drawStr(2, 52, status1);

    // Barre de progression graphique (bas d'écran, 124x7 px)
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

// --- Écran dédié au Refroidissement Forcé (30s) avec grande hélice animée ---
void drawForcedCoolScreen(int secLeft, int progress) {
  u8g.firstPage();
  do {
    // Header (haut gauche)
    u8g.setFont(u8g_font_6x10);
    u8g.drawStrP(2, 12, PSTR("VENTILATION (30s)"));

    // Compte à rebours (haut droite)
    char timeBuf[8];
    sprintf(timeBuf, "%us", secLeft);
    int xPos = 126 - u8g.getStrWidth(timeBuf);
    u8g.drawStr(xPos, 12, timeBuf);

    // Grande icône hélice animée au centre
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
      u8g.drawPixel(cx + 2, cy + 7);
      u8g.drawPixel(cx - 2, cy - 7);
    } else {
      u8g.drawLine(cx - 7, cy - 7, cx + 7, cy + 7);
      u8g.drawLine(cx - 7, cy + 7, cx + 7, cy - 7);
      u8g.drawPixel(cx + 6, cy - 6);
      u8g.drawPixel(cx - 6, cy + 6);
      u8g.drawPixel(cx + 6, cy + 6);
      u8g.drawPixel(cx - 6, cy - 6);
    }

    u8g.drawStrP(12, 47, PSTR("Clic pour arreter"));

    // Barre de progression
    u8g.drawFrame(2, 53, 124, 8);
    int p = constrain(progress, 0, 100);
    if (p > 0) {
      int fillW = map(p, 0, 100, 0, 120);
      if (fillW > 0) u8g.drawBox(4, 55, fillW, 4);
    }
  } while (u8g.nextPage());
}

// --- Écran de fin de soudure avec compteur ---
void drawWaitScreen() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStrP(12, 20, PSTR("Soudure OK !"));

    u8g.setFont(u8g_font_6x10);
    u8g.drawStrP(12, 36, PSTR("Soudure terminee."));

    char countBuf[24];
    sprintf(countBuf, "Total: %u soudures", settings.weldCounter);
    u8g.drawStr(12, 52, countBuf);
  } while (u8g.nextPage());
}

// --- Écran d'alerte sécurité d'urgence (Coupure immédiate) ---
void drawEmergencyScreen(const char* title, const char* reason) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(6, 18, title);

    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(4, 36, reason);
    u8g.drawStrP(4, 52, PSTR("Clic pour acquitter"));
  } while (u8g.nextPage());
}

// --- Affichage dynamique du Menu ---
void drawMenu() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    if (currentMenuPage == PAGE_MAIN) {
      u8g.drawStrP(5, 10, PSTR("--- SOUDEUSE PWM ---"));

      for (int i = 0; i < MAIN_MENU_COUNT; i++) {
        int y = 22 + (i * 10);
        if (i == menuCursor) {
          u8g.drawStr(2, y, ">");
        }
        u8g.drawStrP(12, y, (u8g_pgm_uint8_t*)pgm_read_word(&(mainMenuItems[i])));

        // Température cible du matériau
        if (i < 4) {
          int target = getTargetTemperature(i);
          char buf[8];
          sprintf(buf, "%uC", target);
          u8g.drawStr(100, y, buf);
        }
      }
    } 
    else if (currentMenuPage == PAGE_SETTINGS) {
      u8g.drawStrP(5, 10, PSTR("----* REGLAGES *----"));

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
            if (*valPtr == 1) u8g.drawStrP(106, y, PSTR("OUI"));
            else u8g.drawStrP(106, y, PSTR("NON"));
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
