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

struct APA102Pixel {
  RGB color{};
  std::uint8_t brightness{31};
};

template <std::size_t Count>
class APA102C {
private:
  static constexpr std::size_t EndBytes = (Count + 14) / 16;  // Assumes Polulus end frame findings
  static constexpr std::size_t WireFrameSize =
    4 +             // start frame
    4 * Count +     // LED frames
    4;              // end clocks
  
  int fd_{-1};
  std::array<APA102Pixel, Count> pixels_ = {};
  std::array<std::uint8_t, WireFrameSize> wire_frame_{};

  void rebuild_wire_frame() noexcept;
public:
  explicit APA102C(
    const char* device = "/dev/spidev0.0",
    std::uint32_t speed = 1'000'000);
  ~APA102C();

  APA102Pixel& operator[](std::size_t index) {
    if (index >= Count) throw std::out_of_range("Index out of bounds");
    return pixels_[index];
  }
  const APA102Pixel& operator[](std::size_t index) const {
    if (index >= Count) throw std::out_of_range("Index out of bounds");
    return pixels_[index];
  }
  
  void set_static_color(RGB color);
};
