#pragma once

#include <algorithm>
#include <cmath>

namespace welder {

struct ThermalConfig {
  float target_c = 220.0F;
  float min_c = 20.0F;
  float max_c = 300.0F;
  float kp = 4.0F;
  float ki = 0.08F;
  float kd = 0.35F;
};

class Ntc100kSensor {
 public:
  static float adc_to_celsius(int adc) {
    adc = std::clamp(adc, 1, 1022);
    constexpr float series_resistor = 4700.0F;
    constexpr float nominal_resistance = 100000.0F;
    constexpr float nominal_temp_k = 25.0F + 273.15F;
    constexpr float beta = 3950.0F;
    const float resistance = series_resistor * (1023.0F / static_cast<float>(adc) - 1.0F);
    const float steinhart = std::log(resistance / nominal_resistance) / beta + (1.0F / nominal_temp_k);
    return (1.0F / steinhart) - 273.15F;
  }
};

class PidThermalController {
 public:
  explicit PidThermalController(ThermalConfig config) : cfg_(config) {}

  float step(float current_c, float dt_s) {
    const float error = cfg_.target_c - current_c;
    integral_ = std::clamp(integral_ + error * dt_s, -500.0F, 500.0F);
    const float derivative = dt_s > 0.0F ? (error - prev_error_) / dt_s : 0.0F;
    prev_error_ = error;

    const float pwm = (cfg_.kp * error) + (cfg_.ki * integral_) + (cfg_.kd * derivative);
    return std::clamp(pwm / 1000.0F, 0.0F, 1.0F);
  }

  void set_target(float target_c) { cfg_.target_c = std::clamp(target_c, cfg_.min_c, cfg_.max_c); }
  float target() const { return cfg_.target_c; }

 private:
  ThermalConfig cfg_;
  float integral_ = 0.0F;
  float prev_error_ = 0.0F;
};

}  // namespace welder
