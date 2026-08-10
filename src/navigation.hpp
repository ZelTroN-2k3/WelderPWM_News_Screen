#pragma once

namespace welder {

enum class UiPage { Status, TargetTemp, Safety };
enum class ButtonEvent { None, ShortPress, LongPress };

class SingleButtonNavigator {
 public:
  ButtonEvent update(bool pressed, unsigned now_ms) {
    if (pressed && !pressed_) {
      pressed_ = true;
      pressed_since_ = now_ms;
      long_fired_ = false;
      return ButtonEvent::None;
    }

    if (pressed_ && pressed && !long_fired_ && (now_ms - pressed_since_ >= kLongPressMs)) {
      long_fired_ = true;
      page_ = UiPage::Safety;
      return ButtonEvent::LongPress;
    }

    if (!pressed && pressed_) {
      pressed_ = false;
      if (!long_fired_) {
        page_ = static_cast<UiPage>((static_cast<int>(page_) + 1) % 3);
        return ButtonEvent::ShortPress;
      }
    }

    return ButtonEvent::None;
  }

  UiPage current_page() const { return page_; }

 private:
  static constexpr unsigned kLongPressMs = 1500;
  unsigned pressed_since_ = 0;
  bool pressed_ = false;
  bool long_fired_ = false;
  UiPage page_ = UiPage::Status;
};

}  // namespace welder
