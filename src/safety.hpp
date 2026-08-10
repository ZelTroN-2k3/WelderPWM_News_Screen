#pragma once

namespace welder {

class ThermalRunawayGuard {
 public:
  struct Limits {
    float danger_c = 320.0F;
    float min_rise_c = 1.0F;
    unsigned stall_timeout_ms = 15000;
  };

  ThermalRunawayGuard() = default;
  explicit ThermalRunawayGuard(Limits limits) : limits_(limits) {}

  bool evaluate(bool heater_enabled, float current_c, unsigned now_ms) {
    if (current_c >= limits_.danger_c) {
      tripped_ = true;
      return true;
    }

    if (!heater_enabled) {
      heating_since_ms_ = 0;
      baseline_c_ = current_c;
      return tripped_;
    }

    if (heating_since_ms_ == 0) {
      heating_since_ms_ = now_ms;
      baseline_c_ = current_c;
      return tripped_;
    }

    if ((now_ms - heating_since_ms_) >= limits_.stall_timeout_ms &&
        (current_c - baseline_c_) < limits_.min_rise_c) {
      tripped_ = true;
    }

    return tripped_;
  }

  bool tripped() const { return tripped_; }
  void reset() { tripped_ = false; }

 private:
  Limits limits_;
  unsigned heating_since_ms_ = 0;
  float baseline_c_ = 0.0F;
  bool tripped_ = false;
};

}  // namespace welder
