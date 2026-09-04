#pragma once

#include <Arduino.h>
#include <SPI.h>

class Channel {
private:
  uint8_t cs_pin = 0xFF;          // Chip Select pin for the INA239
  uint8_t sw_pin = 0xFF;          // GPIO pin to switch channel ON/OFF
  float shunt_resistor = 0.0F;    // Shunt resistor value in ohms
  float max_current = 0.0F;       // Max expected current for the channel
  bool state = false;             // state of the channel, enabled/disabled

  uint16_t read_register(uint8_t reg);
  void write_register(uint8_t reg, uint16_t value);

public:
  Channel(
    uint8_t cs_pin,
    uint8_t sw_pin,
    float shunt_resistor = 0.010F,
    float max_current = 10.0F
  );

  struct RawMeasurements {
    int16_t current;
    uint16_t bus_voltage;
    uint32_t power;
    int16_t temperature;
  };
  RawMeasurements read_raw_measurements();

  void begin();

  void enable();
  void disable();

  int16_t get_current();
  uint16_t get_bus_voltage();
  uint32_t get_power();
  int16_t get_die_temperature();
  void request_trigger();
  bool conversion_done();
  bool get_state(); // true means channel is on, false means channel is off
};
