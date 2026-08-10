# 🔥 Automated 3D Filament Welder (PWM & NTC Thermoregulation)

[![Arduino](https://img.shields.io/badge/Platform-Arduino%20Nano%20V3-blue.svg)](https://www.arduino.cc/)
[![Firmware](https://img.shields.io/badge/Version-v3.8%20Alpha-brightgreen.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-orange.svg)]()
[![License](https://img.shields.io/badge/License-Open%20Source-green.svg)]()

> **High-performance modular C++ firmware for 3D filament welding and recycling machines.**  
> Intelligent closed-loop thermal regulation with **NTC 100k** sensor, real-time graphical animated **128x64 OLED HUD**, **EEPROM** parameter storage, active thermal runaway protections, and ergonomic **single-button** navigation.

<p align="center">
  <img src="../News%20Model%20STL.png" alt="3D Welder CAD Model" width="700">
</p>

---

## 👥 Authors & Credits
* **Original Author**: `ptica` (version v3.0)
* **Enhanced Firmware & Features Author**: `ZelTroN2k3` (version v3.8 Alpha)

---

## 🌟 Key Features

### 🌡️ 1. Intelligent Thermal Regulation (Closed Loop)
* **Precise temperature measurement via NTC 100k B3950** on analog pin `A0` with 8x oversampling and Steinhart-Hart equation.
* **Fast Preheating (Initial Boost)**: Increased PWM power during temperature rise to overcome the heater block's thermal inertia.
* **Active Thermal Hold**: Accurate temperature stabilization during the filament fusion/joining phase.
* **Smart Cooling**: The cooling fan runs during the cooling phase and automatically shuts down as soon as the block cools below **45°C**.

### 🎨 2. OLED 128x64 Graphical Interface (HUD & Real-Time Animations)
* **Animated Phase Mini-Icons**:
  * ⚡ **Heating Boost**: Dynamic animated lightning bolt.
  * 🔥 **Target Heating**: Pulsing thermometer.
  * ↔️ **Holding Phase**: Oscillating cursor and guidance arrows for back-and-forth filament sliding.
  * 🌀 **Cooling**: Rotating 4-blade cooling fan.
* **Clean Partitioned HUD**: Material name in large font, target setpoint (`280°C`, `230°C`, `190°C`), countdown timer, and calibrated messages ($\le 21$ chars).
* **Dynamic Progress Bar**: Displays live phase advancement ($25^\circ\text{C} \rightarrow T_{\text{target}}$).
* **Two-Stage Screen Saver**: "Spark Shower" animation after 1 min of inactivity, followed by full OLED sleep after 2 min to prevent burn-in.

### 🕹️ 3. Ergonomic Single-Button Control
* **Main Menu**:
  1. `1. Mode PET` (Target $280^\circ\text{C}$)
  2. `2. Mode PETG` (Target $230^\circ\text{C}$)
  3. `3. Mode PLA` (Target $190^\circ\text{C}$)
  4. `4. Mode Custom` (Customizable target from $150^\circ\text{C}$ to $290^\circ\text{C}$)
  5. `5. [ SETTINGS ]` (Full configuration submenu)
* **Forced Cooling Shortcut (30s)**: Triggered with a quick **triple-click** from the main menu to cool down the heater before storage.

### ⚙️ 4. Settings Submenu & EEPROM Persistence
* Configure safety timeouts and durations for holding / cooling / fan run time.
* `Custom Target Temp`: Adjust custom temperature setpoint in steps of $5^\circ\text{C}$.
* `Total Welds`: Persistent statistic counter of completed welds.
* `Screen Inversion`: Switch between standard theme (Black Background) and inverted theme (White Background / Black Text).
* `! Factory Reset`: Instant restore of factory defaults via a double-click.

### 🛡️ 5. Active Thermal Protections
* **Disconnected / Short-Circuited Sensor Detection**: Instant heater shutdown and `NTC SENSOR ERROR` warning.
* **Thermal Runaway Protection**: Emergency heater cutoff if $T > 295^\circ\text{C}$ with `OVERHEAT ALERT !` warning.
* **User Emergency Stop**: Single click during welding instantly aborts heating and starts cooling fan.

### ⚡ 6. Memory Optimized Architecture
* Display strings and menus stored in Flash (`PROGMEM` / `PSTR`), preserving **over 950 bytes of free SRAM** (only ~53% SRAM used on ATmega328P).

---

## 📐 Wiring Diagram (Arduino Nano V3)

| Component                 | Component Pin                        | Arduino Nano V3 Pin                | Notes                                                           |
| :------------------------ | :----------------------------------- | :--------------------------------- | :-------------------------------------------------------------- |
| **Push Button**           | Terminal 1 / Terminal 2              | **D10** / **GND**                  | Uses internal `INPUT_PULLUP`                                    |
| **Heater (MOSFET)**       | Gate (via 100Ω resistor)             | **D3** (PWM)                       | Heater cartridge PWM control                                    |
| **Cooling Fan 5V/12V**    | Control (Transistor/MOSFET)          | **D2**                             | Forced cooling & cycle completion                               |
| **Red LED**               | Anode (+ via resistor) / Cathode     | **D8** / **GND**                   | Heating status & alarms                                         |
| **Green LED**             | Anode (+ via resistor) / Cathode     | **D6** / **GND**                   | System ready & cooling indicator                                |
| **NTC 100k Sensor**       | Terminal 1 / Terminal 2              | **A0** / **GND**                   | Voltage divider with $47\,\text{k}\Omega$ resistor to **+5V**   |
| **OLED Display I2C**      | SDA / SCL / VCC / GND                | **A4** / **A5** / **5V** / **GND** | SSD1306, SSD1309 or SH1106 (128x64)                             |
| **Piezo Buzzer (Opt.)**   | (+) / (-)                            | **D4** / **GND**                   | Enabled via `#define ENABLE_BUZZER 1`                           |

### NTC 100k Thermistor Circuit (Pin A0)

```
                         +5V (Arduino)
                            │
                        [ 47 kΩ ]  (Pull-up Resistor 1/4W)
                            │
     Arduino Pin A0 ────────┼──────────────────────┐
     (Analog Input)         │                      │
                       [ NTC 100k ]           [ 100 nF ] (Optional filter cap)
                            │                      │
                           GND ────────────────────┘
```

---

## 🕹️ Single-Button Operating Guide

```
┌─────────────────────────────────────────────────────────────┐
│                        SINGLE BUTTON                        │
├──────────────────────────────┬──────────────────────────────┤
│ IN MAIN MENU                 │ IN SETTINGS SUBMENU          │
│ • 1 Click  : Move cursor     │ • 1 Click  : Move cursor     │
│ • 2 Clicks : Start weld      │ • 2 Clicks : Edit parameter  │
│ • 3 Clicks : Forced Cooling  │                              │
├──────────────────────────────┴──────────────────────────────┤
│ DURING WELDING OR ACTIVE ALARM                              │
│ • 1 Click  : Emergency Stop / Dismiss Alarm                 │
└─────────────────────────────────────────────────────────────┘
```

---

## 📺 Graphical OLED Screens Overview

### 1. Welcome HUD Screen
```
┌──                                                    ──┐
│                                                        │
│                        WELDER                          │
│                  ptica & ZelTroN2k3                    │
│                      v3.8 Alpha                        │
│                                                        │
└──                                                    ──┘
```

### 2. Main Menu (Material & Temperature Selection)
```
┌────────────────────────────────────────────────────────┐
│ --- PWM WELDER ---                                     │
│ > 1. Mode PET                                     280C │
│   2. Mode PETG                                    230C │
│   3. Mode PLA                                     190C │
│   4. Mode Custom                                  200C │
│   5. [ SETTINGS ]                                      │
└────────────────────────────────────────────────────────┘
```

### 3. Settings Menu (EEPROM Configuration)
```
┌────────────────────────────────────────────────────────┐
│ ----* SETTINGS *----                                   │
│ > Heat PET                                         35  │
│   Heat PETG                                        25  │
│   Heat PLA                                         25  │
│   Target Temp Cust                                200C │
│   < Save & Exit                                        │
└────────────────────────────────────────────────────────┘
```

### 4. Welding Screen (Heating & Real-Time Temperature)
```
┌────────────────────────────────────────────────────────┐
│ Mode 1         │       [ 🔥 ]       │              25s │  <-- Countdown timer
├────────────────────────────────────────────────────────┤
│                                                        │
│  PET                                             280°C │  <-- Material & Target Temperature
│                                                        │
├────────────────────────────────────────────────────────┤
│ Heating : 195°C                                        │  <-- Real-time measured temperature
│ [████████████████████░░░░░░░░░░]                       │  <-- Dynamic progress (25°C -> 280°C)
└────────────────────────────────────────────────────────┘
```

### 5. Forced Cooling Screen (30s - Triple-Click Shortcut)
```
┌────────────────────────────────────────────────────────┐
│ COOLING (30s)                                      28s │  <-- Countdown timer (30s -> 0s)
│                                                        │
│                        ( 🌀 )                          │  <-- Large animated fan
│                                                        │
│ Click to stop                                          │
│ [████████████░░░░░░░░░░░░░░░░░░]                       │  <-- Cooling progression
└────────────────────────────────────────────────────────┘
```

### 6. Welding Finished Screen (Statistic Counter)
```
┌────────────────────────────────────────────────────────┐
│ Weld OK !                                              │
│                                                        │
│ Welding complete.                                      │
│ Total: 48 welds                                        │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 7. Thermal Safety Alert Screen (Emergency Cutoff)
```
┌────────────────────────────────────────────────────────┐
│ OVERHEAT ALERT                                         │
│                                                        │
│ Temperature > 295C                                     │
│ Click to dismiss                                       │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 8. Screen Saver (Animated Spark Shower at 1 min)
```
┌────────────────────────────────────────────────────────┐
│                      .       *        .                │
│                     PWM WELDER                         │
│                     [ Sleep ]                          │
│           *                     .            *         │
│                   .             *                      │
└────────────────────────────────────────────────────────┘
```

---

## 📂 Project Architecture & Multi-Language Versions

This project is fully maintained across **3 synchronized language versions**:

```
WelderPWM_new_3+Screen/
├── README.md                          # Main project overview
├── WelderPWM_new_3_Screen_FR/         # 🇫🇷 French Version (French UI & Logs)
│   ├── README.md                      # French documentation
│   ├── WelderPWM_new_3_Screen_FR.ino
│   └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
├── WelderPWM_new_3_Screen_EN/         # 🇬🇧 English Version (Full English UI & Logs)
│   ├── README.md                      # English documentation
│   ├── WelderPWM_new_3_Screen_EN.ino
│   └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
└── WelderPWM_new_3_Screen_RU/         # 🇷🇺 Russian Version (Russian UI & Menus)
    ├── README.md                      # Russian documentation
    ├── WelderPWM_new_3_Screen_RU.ino
    └── Config.h, Globals.h/.cpp, Hardware.h/.cpp, Display.h/.cpp, MenuLogic.h/.cpp, ScreenSaver.h/.cpp
```

---

## 🛠️ Installation & Flashing

### 1. Prerequisites (Arduino Libraries)
Install the following libraries via the Arduino IDE Library Manager:
* **`U8glib`** (by *oliver*): Fast I2C OLED display driver.
* **`EncButton`** (by *AlexGyver*): Single button handler (click, double-click, triple-click).
* **`Wire`** and **`EEPROM`**: Built-in with Arduino AVR core.

### 2. Arduino IDE Configuration
* **Board**: `Arduino Nano`
* **Processor**: `ATmega328P` (or `ATmega328P (Old Bootloader)` depending on your Nano board)
* **Port**: Select the appropriate COM port.

### 3. Command Line Compilation (Optional with `arduino-cli`)
```bash
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old .
arduino-cli upload -p COM3 --fqbn arduino:avr:nano:cpu=atmega328old .
```

---

## 📄 License
Open-source project distributed under the MIT / Open Source license. Free for personal use, modifications, and continuous improvement by the 3D printing community.
