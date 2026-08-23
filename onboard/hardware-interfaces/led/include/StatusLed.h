#pragma once

#include <utility>
#include <LedTypes.h>

template <typename Driver>
class StatusLed {
private:
  Driver driver_;
public:
  template <typename... Args>
  explicit StatusLed(Args&&... args)
    : driver_(std::forward<Args>(args)...) {}

  void set_state(LEDStates state);
};