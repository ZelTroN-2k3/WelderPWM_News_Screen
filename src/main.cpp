#include <iostream>

#include "hud.hpp"
#include "navigation.hpp"
#include "safety.hpp"
#include "storage.hpp"
#include "thermal.hpp"

namespace welder {

class Firmware {
 public:
  Firmware() : thermal_(load_config()) {}

  void tick(int adc, bool button_pressed, unsigned now_ms, float dt_s) {
    nav_.update(button_pressed, now_ms);
    const float current_c = Ntc100kSensor::adc_to_celsius(adc);
    float pwm = thermal_.step(current_c, dt_s);
    const bool runaway = safety_.evaluate(pwm > 0.05F, current_c, now_ms);
    if (runaway) {
      pwm = 0.0F;
    }

    hud_.render({current_c, thermal_.target(), pwm, runaway}, now_ms / 150);
    pwm_output_ = pwm;
  }

  void set_target(float target_c) {
    thermal_.set_target(target_c);
    const float persisted = thermal_.target();
    store_.write(0, persisted);
  }

  const OledHud128x64& hud() const { return hud_; }
  float pwm_output() const { return pwm_output_; }

 private:
  ThermalConfig load_config() {
    ThermalConfig cfg;
    float target = cfg.target_c;
    if (store_.read(0, target)) {
      cfg.target_c = target;
    } else {
      store_.write(0, cfg.target_c);
    }
    return cfg;
  }

  EepromStore store_;
  PidThermalController thermal_;
  ThermalRunawayGuard safety_;
  SingleButtonNavigator nav_;
  OledHud128x64 hud_;
  float pwm_output_ = 0.0F;
};

}  // namespace welder

int main() {
  welder::Firmware fw;
  fw.set_target(235.0F);

  for (unsigned t = 100; t <= 2000; t += 100) {
    fw.tick(380 + static_cast<int>(t / 200), false, t, 0.1F);
  }

  for (const auto& line : fw.hud().frame()) {
    if (!line.empty()) {
      std::cout << line << '\n';
    }
  }

  return 0;
}
