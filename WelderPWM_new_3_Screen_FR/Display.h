#ifndef DISPLAY_H
#define DISPLAY_H

#include "Globals.h"
#include "Hardware.h"

// Écran d'accueil avec cadre HUD et version
void drawWelcomeScreen();

// Écran de notification éphémère
void drawMessageScreen(const char* line1, const char* line2);

// Dessin des icônes animées de chaque phase (⚡, 🔥, ↔️, 🌀)
void drawPhaseIcon(int phaseMode, int frame, bool isBoost);

// Affichage graphique du matériau, compte à rebours, icône et barre de progression (Design Cloisonné)
void drawMaterialScreen(int modeIdx, int secondsLeft, int progressPercent, int phaseMode, bool isBoost, const char* status1);

// Écran dédié au Refroidissement Forcé (30s) avec grande hélice animée
void drawForcedCoolScreen(int secLeft, int progress);

// Écran de fin de soudure avec compteur statistique
void drawWaitScreen();

// Écran d'alerte de sécurité thermique d'urgence
void drawEmergencyScreen(const char* title, const char* reason);

// Affichage dynamique du Menu Principal et Menu Réglages
void drawMenu();

#endif // DISPLAY_H
