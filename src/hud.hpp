#pragma once

#include <array>
#include <cstdlib>
#include <string>

namespace welder {

class OledHud128x64 {
 public:
  struct Telemetry {
    float current_c = 0.0F;
    float target_c = 0.0F;
    float pwm = 0.0F;
    bool thermal_fault = false;
  };

  void render(const Telemetry& telemetry, unsigned tick) {
    static constexpr std::array<const char*, 4> spinner{"|", "/", "-", "\\"};
    frame_[0] = std::string("WELDER HUD 128x64 ") + spinner[tick % spinner.size()];
    frame_[1] = "TEMP  : " + value(telemetry.current_c) + " C";
    frame_[2] = "TARGET: " + value(telemetry.target_c) + " C";
    frame_[3] = "HEAT  : " + value(telemetry.pwm * 100.0F) + " %";
    frame_[4] = telemetry.thermal_fault ? "SAFETY: THERMAL RUNAWAY" : "SAFETY: OK";
  }

  const std::array<std::string, 8>& frame() const { return frame_; }

 private:
  static std::string value(float v) {
    const auto rounded = static_cast<int>(v * 10.0F);
    return std::to_string(rounded / 10) + "." + std::to_string(std::abs(rounded % 10));
  }

  std::array<std::string, 8> frame_{};
};

}  // namespace welder
