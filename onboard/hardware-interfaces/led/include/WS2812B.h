// Done through PWM
#include <cstdint>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <array>

#include "LedTypes.h"
#include "ws2811.h"

constexpr int TargetFreq =          WS2811_TARGET_FREQ;
constexpr int GPIOPin =             18;
constexpr int Dma =                 10;
constexpr int StripType =           WS2811_STRIP_GRB;

struct WS2812BPixel {
  RGB color{};
};

template <std::size_t Count>
class WS2812B {
private:
  std::array<WS2812BPixel, Count> pixels_ = {};
  ws2811_t ledstring_;
  void rebuild_wire_frame() noexcept;
public:
  WS2812B();
  ~WS2812B();

  WS2812BPixel& operator[](std::size_t index) {
    if (index >= Count) throw std::out_of_range("Index out of bounds");
    return pixels_[index];
  }
  const WS2812BPixel& operator[](std::size_t index) const {
    if (index >= Count) throw std::out_of_range("Index out of bounds");
    return pixels_[index];
  }
  
  void set_static_color(RGB color);
};
