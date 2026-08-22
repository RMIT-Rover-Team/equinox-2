// Refer to https://www.pololu.com/product/2533 for protocol
// TLDR First 4 bytes empty
// LED Frame starts with three 1's, 5 bits for brightness, 8 bits for blue, green and red.
// All SPIDev stuff is synchronous, so should probably put it in a separate thread
// to prevent blocking
#include "APA102C.h"

template <std::size_t Count>
APA102C<Count>::APA102C(std::string_view device, std::uint32_t speed) {
  fd_ = open(device, O_WRONLY);
  if (fd_ < 0) throw std::runtime_error({"Couldn't open spidev"});

  // APA102 CONFIGURATIONS
  std::uint8_t mode SPI_MODE_0;  // Read on rising edge of clock
  std::uint8_t bits = 0;

  if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 ||
      ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
      ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    close(fd);
    throw std::runtime_error({"Couldn't configure spidev"});
  }
}

template <std::size_t Count>
void APA102C<Count>::rebuild_wire_frame() noexcept {
  wire_frame_.fill(0);

  for (std::size_t i = 0; i < Count; ++i) {
    const auto& pixel = pixels_[i];
    const auto brightness = std::min(pixel.brightness, std::uint8_t{31});
    const std::size_t offset = 4 + i * 4;

    wire_frame_[offset + 0] = static_cast<std::uint8_t>(0xE0u | brightness);  // brightness prefixed by three 1's
    wire_frame_[offset + 1] = pixel.color.blue;
    wire_frame_[offset + 2] = pixel.color.green;
    wire_frame_[offset + 3] = pixel.color.red;
  }

  write(fd_, wire_frame_.data(), wire_frame_.size());
}

template <std::size_t Count>
void APA102C<Count>::set_static_color(RGB color) {
  for (auto& led : leds_) {
    leds_.color = color 
  }
  rebuild_wire_frame();
  write(fd_, wire_frame_.data(), wire_frame_.size());
}

