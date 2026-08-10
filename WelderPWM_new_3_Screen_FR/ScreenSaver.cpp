#include "ScreenSaver.h"

// Initialisation des étincelles avec des positions et vitesses aléatoires
void initSparks() {
  for (int i = 0; i < NUM_SPARKS; i++) {
    sparks[i].x = random(0, 128);
    sparks[i].y = random(0, 64);
    sparks[i].speedY = random(1, 4);
    sparks[i].speedX = random(-1, 2);
  }
}

// Mise à jour de la physique des étincelles volantes (montée du bas vers le haut)
void updateSparks() {
  for (int i = 0; i < NUM_SPARKS; i++) {
    sparks[i].y -= sparks[i].speedY;
    sparks[i].x += sparks[i].speedX;
    if (sparks[i].y < 0 || sparks[i].x < 0 || sparks[i].x > 127) {
      sparks[i].y = 63;
      sparks[i].x = random(0, 128);
      sparks[i].speedY = random(1, 4);
      sparks[i].speedX = random(-1, 2);
    }
  }
}

// Rendu graphique de l'économiseur d'écran (Texte central + Étincelles volantes)
void drawSparkScreenSaver() {
  u8g.firstPage();
  do {
    // Titre au centre
    u8g.setFont(u8g_font_7x14B);
    int titleX = (128 - u8g.getStrWidth("SOUDEUSE PWM")) / 2;
    u8g.drawStrP(titleX, 28, PSTR("SOUDEUSE PWM"));

    u8g.setFont(u8g_font_6x10);
    int subX = (128 - u8g.getStrWidth("[ Veille ]")) / 2;
    u8g.drawStrP(subX, 44, PSTR("[ Veille ]"));

    // Dessin des étincelles volantes (pixels animés)
    for (int i = 0; i < NUM_SPARKS; i++) {
      u8g.drawPixel(sparks[i].x, sparks[i].y);
      if (i % 2 == 0) { // Étincelles doublées pour effet visuel brillant
        u8g.drawPixel(sparks[i].x + 1, sparks[i].y);
        u8g.drawPixel(sparks[i].x, sparks[i].y + 1);
      }
    }
  } while (u8g.nextPage());
}
