#include "StatusLed.h"

template <typename Driver>
void StatusLed<Driver>::set_state(LEDStates state) {
  driver_.set_static_color(get_static_color(state));
}