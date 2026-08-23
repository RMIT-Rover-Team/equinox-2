#pragma once

struct RGB {
  uint8_t red{}, green{}, blue{};
};

enum LEDStates {
  Safe = 0,
  Motion = 1,
  AutoPrep = 2,
  Auto = 3,
  Locked = 4,
  Conflict = 5,
  Error = 6
};

namespace LedColors {
  inline constexpr RGB White   {255, 255, 255};  // safe
  inline constexpr RGB Blue    {  0,   0, 255};  // led motion
  inline constexpr RGB Cyan    {  0, 255, 255};  // auto prep
  inline constexpr RGB Green   {  0, 128,   0};  // auto
  inline constexpr RGB Yellow  {255, 255,   0};  // locked
  inline constexpr RGB Magenta {255,   0, 255};  // conflict
  inline constexpr RGB Red     {255,   0,   0};  // error
  inline constexpr RGB Black   {  0,   0,   0};  // default
};

inline RGB get_state_color(LEDStates state) { 
  switch(state) {
    case Safe:
      return LedColors::White;
    case Motion:
      return LedColors::Blue;
    case AutoPrep:
      return LedColors::Cyan;
    case Auto:
      return LedColors::Green;
    case Locked:
      return LedColors::Yellow;
    case Conflict:
      return LedColors::Magenta;
    case Error:
      return LedColors::Red;
    default:
      return LedColors::Black;
  }
};