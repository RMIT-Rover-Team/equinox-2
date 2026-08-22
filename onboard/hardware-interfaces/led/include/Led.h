#pragma once
#include "stdint.h"

struct RGB {
  uint8_t red{}, green{}, blue{};
};

enum LEDStates{
  Safe = 0,
  Motion = 1,
  AutoPrep = 2,
  Auto = 3,
  Locked = 4,
  Conflict = 5,
  Error = 6
};
enum LEDColors{
  White,    // SAFE
  Blue,     // LED MOTION
  Cyan,     // LED AUTO PREP
  Green,    // LED AUTO
  Yellow,   // LED LOCKED
  Magenta,  // LED CONFLICT
  Red,      // LED ERROR
  Black,    // LED DEFAULT
};

class Led {
  Led();
  ~Led();
};