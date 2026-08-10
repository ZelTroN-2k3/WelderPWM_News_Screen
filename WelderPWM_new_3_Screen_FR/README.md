# 🔥 Soudeuse de Filament 3D Automatisée (PWM & Régulation CTN)

[![Arduino](https://img.shields.io/badge/Platform-Arduino%20Nano%20V3-blue.svg)](https://www.arduino.cc/)
[![Firmware](https://img.shields.io/badge/Version-v3.8%20Alpha-brightgreen.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-orange.svg)]()
[![License](https://img.shields.io/badge/License-Open%20Source-green.svg)]()

> **Firmware modulaire C++ haute performance pour soudeuse/recycleuse de filament 3D.**  
> Régulation thermique intelligente en boucle fermée par sonde **CTN 100k**, écran **OLED 128x64** avec animations graphiques temps réel, mémorisation **EEPROM**, sécurités thermiques actives et contrôle ergonomique par **bouton unique**.

<p align="center">
  <img src="../News%20Model%20STL.png" alt="Modélisation 3D Soudeuse" width="700">
</p>

---

## 👥 Auteurs & Crédits
* **Auteur original** : `ptica` (version v3.0)
* **Auteur de la version évoluée & améliorations** : `ZelTroN2k3` (version v3.8 Alpha)

---

## 🌟 Fonctionnalités Principales

### 🌡️ 1. Régulation Thermique Intelligente (Boucle Fermée)
* **Mesure précise par CTN 100k B3950** sur broche analogique `A0` avec sur-échantillonnage 8x et équation de Steinhart-Hart.
* **Préchauffage rapide (Boost initial)** : Puissance PWM accrue pendant la montée en température pour vaincre l'inertie thermique du bloc.
* **Maintien thermique actif** : Stabilisation exacte de la température pendant la phase de fusion/pression du filament.
* **Refroidissement intelligent** : Le ventilateur tourne pendant la phase de refroidissement et s'arrête automatiquement dès que le bloc repasse sous **$45^\circ\text{C}$**.

### 🎨 2. Interface Graphique OLED 128x64 (HUD & Animations)
* **Mini-icônes animées selon la phase** :
  * ⚡ **Boost de chauffe** : Éclair animé dynamique.
  * 🔥 **Chauffe d'approche** : Thermomètre pulsant.
  * ↔️ **Maintien** : Curseur oscillant et flèches de guidage pour le va-et-vient du filament.
  * 🌀 **Refroidissement** : Hélice de ventilateur tournante à 4 pales.
* **Affichage cloisonné moderne** : Nom du matériau en grand format, consigne cible (`280°C`, `230°C`, `190°C`), compte à rebours et messages calibrés ($\le 21$ caractères).
* **Barre de progression dynamique** : Indique en direct le pourcentage d'avancement de la phase ($25^\circ\text{C} \rightarrow T_{\text{cible}}$).
* **Économiseur d'écran en 2 étapes** : Animation de « Pluie d'étincelles » après 1 min d'inactivité, puis extinction totale de l'écran OLED après 2 min pour préserver la dalle.

### 🕹️ 3. Navigation Ergonomique à Bouton Unique
* **Menu Principal** :
  1. `1. Mode PET` (Cible $280^\circ\text{C}$)
  2. `2. Mode PETG` (Cible $230^\circ\text{C}$)
  3. `3. Mode PLA` (Cible $190^\circ\text{C}$)
  4. `4. Mode Custom` (Cible personnalisable de $150^\circ\text{C}$ à $290^\circ\text{C}$)
  5. `5. [ REGLAGES ]` (Sous-menu de configuration complète)
* **Raccourci Refroidissement Forcé (30s)** : Déclenchable par un simple **triple-clic** depuis le menu principal pour refroidir l'appareil avant rangement.

### ⚙️ 4. Menu Réglages & Sauvegarde EEPROM
* Réglage des timeouts de sécurité et durées de maintien / refroidissement / ventilateur.
* `Temp. Custom` : Réglage de la consigne du mode Custom par pas de $5^\circ\text{C}$.
* `Total Soudures` : Compteur statistique persistant du nombre total de soudures réussies.
* `Inversion Ecran` : Bascule entre thème standard (Fond Noir) et thème inversé (Fond Blanc / Texte Noir).
* `! Reset Usine` : Restauration immédiate des paramètres d'usine en un double-clic.

### 🛡️ 5. Sécurités Thermiques Actives
* **Protection sonde débranchée / court-circuit** : Coupure instantanée du MOSFET et alerte `ERREUR SONDE CTN`.
* **Protection surchauffe (*Thermal Runaway*)** : Coupure d'urgence immédiate si $T > 295^\circ\text{C}$ avec alerte `ALERTE SURCHAUFFE !`.
* **Arrêt d'urgence utilisateur** : Un simple clic pendant la soudure interrompt immédiatement la chauffe et ventile le bloc.

### ⚡ 6. Empreinte Mémoire Ultra-Optimisée
* Textes et menus stockés en mémoire Flash (`PROGMEM` / `PSTR`), libérant **plus de 950 octets de RAM libre** (seulement 53% de RAM utilisée sur ATmega328P).

---

## 📐 Schéma de Câblage (Arduino Nano V3)

| Composant                 | Broche du composant                  | Broche Arduino Nano V3             | Notes                                                           |
| :------------------------ | :----------------------------------- | :--------------------------------- | :-------------------------------------------------------------- |
| **Bouton Poussoir**       | Borne 1 / Borne 2                    | **D10** / **GND**                  | Utilise le `INPUT_PULLUP` interne                               |
| **Chauffage (MOSFET)**    | Grille (Gate via 100Ω)               | **D3** (PWM)                       | Commande du bloc de chauffe                                     |
| **Ventilateur 5V/12V**    | Commande (Transistor/MOSFET)         | **D2**                             | Refroidissement forcé et fin de cycle                           |
| **LED Rouge**             | Anode (+ via résistance) / Cathode   | **D8** / **GND**                   | Témoin de chauffe & alarmes                                     |
| **LED Verte**             | Anode (+ via résistance) / Cathode   | **D6** / **GND**                   | Témoin système prêt & refroidissement                           |
| **Sonde CTN 100k**        | Borne 1 / Borne 2                    | **A0** / **GND**                   | Pont diviseur avec résistance $47\,\text{k}\Omega$ vers **+5V** |
| **Écran OLED I2C**        | SDA / SCL / VCC / GND                | **A4** / **A5** / **5V** / **GND** | SSD1306, SSD1309 ou SH1106 (128x64)                             |
| **Buzzer Piezo (Option)** | Pôle (+) / Pôle (-)                  | **D4** / **GND**                   | Activable via `#define ENABLE_BUZZER 1`                         |

### Schéma de branchement de la sonde CTN 100k (A0)

```
                         +5V (Arduino)
                           │
                       [ 47 kΩ ]  (Résistance Pull-up 1/4W)
                           │
    Broche A0 Arduino ─────┼──────────────────────┐
    (Entrée mesure)        │                      │
                      [ CTN 100k ]           [ 100 nF ] (Optionnel, filtrage)
                           │                      │
                          GND ────────────────────┘
```

---

## 🕹️ Guide d'Utilisation du Bouton Unique

```
┌─────────────────────────────────────────────────────────────┐
│                       BOUTON UNIQUE                         │
├──────────────────────────────┬──────────────────────────────┤
│ DANS LE MENU PRINCIPAL       │ DANS LE MENU RÉGLAGES        │
│ • 1 Clic   : Déplacer curseur│ • 1 Clic   : Déplacer curseur│
│ • 2 Clics  : Lancer soudure  │ • 2 Clics  : Éditer paramètre│
│ • 3 Clics  : Ventilo Forcé   │                              │
├──────────────────────────────┴──────────────────────────────┤
│ EN COURS DE SOUDURE OU ALARME                               │
│ • 1 Clic   : Arrêt d'urgence / Acquittement de l'alarme     │
└─────────────────────────────────────────────────────────────┘
```

---

## 📺 Aperçu des Écrans Graphiques

### 1. Écran d'Accueil (HUD Technologique)
```
┌──                                                    ──┐
│                                                        │
│                       SOUDEUSE                         │
│                  ptica & ZelTroN2k3                    │
│                      v3.8 Alpha                        │
│                                                        │
└──                                                    ──┘
```

### 2. Menu Principal (Sélection du Matériau & Température)
```
┌────────────────────────────────────────────────────────┐
│ --- SOUDEUSE PWM ---                                   │
│ > 1. Mode PET                                     280C │
│   2. Mode PETG                                    230C │
│   3. Mode PLA                                     190C │
│   4. Mode Custom                                  200C │
│   5. [ REGLAGES ]                                      │
└────────────────────────────────────────────────────────┘
```

### 3. Menu Réglages (Configuration EEPROM)
```
┌────────────────────────────────────────────────────────┐
│ ----* REGLAGES *----                                   │
│ > Chauffe PET                                      35  │
│   Chauffe PETG                                     25  │
│   Chauffe PLA                                      25  │
│   Temp. Custom                                    200C │
│   < Sauver & Retour                                    │
└────────────────────────────────────────────────────────┘
```

### 4. Écran de Soudure (Chauffe & Régulation en direct)
```
┌────────────────────────────────────────────────────────┐
│ Mode 1         │       [ 🔥 ]       │              25s │  <-- Compte à rebours
├────────────────────────────────────────────────────────┤
│                                                        │
│  PET                                             280°C │  <-- Matériau & Température Cible
│                                                        │
├────────────────────────────────────────────────────────┤
│ Chauffe : 195°C                                        │  <-- Température mesurée en temps réel
│ [████████████████████░░░░░░░░░░]                       │  <-- Progression (25°C -> 280°C)
└────────────────────────────────────────────────────────┘
```

### 5. Écran de Ventilation Forcée (30s - Raccourci Triple-Clic)
```
┌────────────────────────────────────────────────────────┐
│ VENTILATION (30s)                                  28s │  <-- Compte à rebours (30s -> 0s)
│                                                        │
│                        ( 🌀 )                          │  <-- Grande hélice animée
│                                                        │
│ Clic pour arreter                                      │
│ [████████████░░░░░░░░░░░░░░░░░░]                       │  <-- Progression du refroidissement
└────────────────────────────────────────────────────────┘
```

### 6. Écran de Fin de Soudure (Compteur Statistique)
```
┌────────────────────────────────────────────────────────┐
│ Soudure OK !                                           │
│                                                        │
│ Soudure terminee.                                      │
│ Total: 48 soudures                                     │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 7. Écran d'Alerte Sécurité Thermique (Coupure d'Urgence)
```
┌────────────────────────────────────────────────────────┐
│ ALERTE SURCHAUFFE                                      │
│                                                        │
│ Temperature > 295C                                     │
│ Clic pour acquitter                                    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 8. Écran de Veille (Pluie d'étincelles animée à 1 min)
```
┌────────────────────────────────────────────────────────┐
│                      .       *        .                │
│                   SOUDEUSE PWM                         │
│                     [ Veille ]                         │
│           *                     .            *         │
│                   .             *                      │
└────────────────────────────────────────────────────────┘
```

---

## 📂 Architecture des Dossiers & Versions Multi-Langues

Le projet est disponible en **3 langues complètes et synchronisées** :

```
WelderPWM_new_3+Screen/
├── README.md                          # Documentation officielle du projet
├── WelderPWM_new_3_Screen_FR/         # 🇫🇷 Version Française (Interface & Logs en Français)
│   ├── README.md                      # Documentation en français
│   ├── WelderPWM_new_3_Screen_FR.ino
│   └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
├── WelderPWM_new_3_Screen_EN/         # 🇬🇧 English Version (Full English UI & Logs)
│   ├── README.md                      # English documentation
│   ├── WelderPWM_new_3_Screen_EN.ino
│   └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
└── WelderPWM_new_3_Screen_RU/         # 🇷🇺 Русская Версия (Интерфейс и меню на русском языке)
    ├── README.md                      # Русскоязычная документация
    ├── WelderPWM_new_3_Screen_RU.ino
    └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
```

---

## 🛠️ Installation & Téléversement

### 1. Prérequis (Bibliothèques Arduino)
Installez les bibliothèques suivantes via le Gestionnaire de bibliothèques Arduino IDE :
* **`U8glib`** (par *oliver*) : Pilote d'affichage OLED I2C rapide.
* **`EncButton`** (par *AlexGyver*) : Gestion avancée du bouton unique (clic, double-clic, triple-clic).
* **`Wire`** et **`EEPROM`** : Incluses de base dans le noyau Arduino AVR.

### 2. Configuration Arduino IDE
* **Type de carte** : `Arduino Nano`
* **Processeur** : `ATmega328P` (ou `ATmega328P (Old Bootloader)` selon votre clone Nano)
* **Port** : Sélectionnez le port COM correspondant.

### 3. Compilation avec `arduino-cli` (Optionnel)
```bash
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old .
arduino-cli upload -p COM3 --fqbn arduino:avr:nano:cpu=atmega328old .
```

---

## 📄 Licence
Projet open-source distribué sous licence MIT / Open Source. Libre pour utilisation personnelle, modifications et amélioration continue par la communauté d'impression 3D.
