// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// 24 bit frame, GRB
// USING PCM ON PIN 11
#include "WS2812B.h"



template <std::size_t Count>
WS2812B<Count>::WS2812B() {
  ledstring_ =
  {
    .freq = TargetFreq,
    .dmanum = Dma,
    .channel =
    {
      [0] =
      {
        .gpionum = GPIOPin,
        .invert = 0,
        .count = Count,
        .strip_type = StripType,
        .brightness = 255,
      },
      [1] =
      {
        .gpionum = 0,
        .invert = 0,
        .count = 0,
        .brightness = 0,
      },
    },
  };

  ws2811_init(&ledstring_);
}

template <std::size_t Count>
void WS2812B<Count>::rebuild_wire_frame() noexcept {
  for (std::size_t i = 0; i < Count; ++i) {
    ledstring_.channel[0].leds[i] = static_cast<ws2811_led_t>(pack_rgb(pixels_[i].color.red, pixels_[i].color.green, pixels_[i].color.blue));
  }
}

constexpr ws2811_led_t pack_rgb(
  std::uint8_t red,
  std::uint8_t green,
  std::uint8_t blue) noexcept
{
  return  (static_cast<ws2811_led_t>(red)   << 16) |
          (static_cast<ws2811_led_t>(green) << 8)  |
           static_cast<ws2811_led_t>(blue);
}

template <std::size_t Count>
void WS2812B<Count>::set_static_color(RGB color) {
  for (auto& pixel : pixels_) {
    pixel.color = color;
  }
  rebuild_wire_frame();
  ws2811_render(&ledstring_);
}
