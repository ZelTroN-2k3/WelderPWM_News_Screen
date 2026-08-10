#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace welder {

class EepromStore {
 public:
  static constexpr std::size_t kSize = 256;

  template <typename T>
  bool write(std::size_t address, const T& value) {
    if (address + sizeof(T) > bytes_.size()) {
      return false;
    }
    std::memcpy(bytes_.data() + address, &value, sizeof(T));
    return true;
  }

  template <typename T>
  bool read(std::size_t address, T& value) const {
    if (address + sizeof(T) > bytes_.size()) {
      return false;
    }
    std::memcpy(&value, bytes_.data() + address, sizeof(T));
    return true;
  }

 private:
  std::array<std::uint8_t, kSize> bytes_{};
};

}  // namespace welder
