# WelderPWM_News_Screen

High-performance modular C++ firmware skeleton for 3D filament welding and recycling machines.

Implemented modules:
- NTC 100k closed-loop thermal regulation (`src/thermal.hpp`)
- Active thermal runaway protection (`src/safety.hpp`)
- Animated 128x64 OLED HUD renderer (`src/hud.hpp`)
- EEPROM-style parameter storage abstraction (`src/storage.hpp`)
- Ergonomic single-button navigation state machine (`src/navigation.hpp`)
- Integrated firmware loop (`src/main.cpp`)

## Quick build check

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o welder_firmware_demo
./welder_firmware_demo
```
