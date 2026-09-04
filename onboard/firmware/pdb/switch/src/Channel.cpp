#include "Channel.h"
#include "BoardConfig.h"

// Constructor
Channel::Channel(
  uint8_t cs,
  uint8_t sw,
  float shunt,
  float max_i
)
  : cs_pin(cs),
    sw_pin(sw),
    shunt_resistor(shunt),
    max_current(max_i),
    state(false)
{}

void Channel::begin()
{

  // --- CS pin setup ---
  switch (cs_pin) {
    case PdbConfig::ChannelCsPb6:
      DDRB |= _BV(PB6);
      PORTB |= _BV(PB6);        // deselect
      break;
    case PdbConfig::ChannelCsPb7:
      DDRB |= _BV(PB7);
      PORTB |= _BV(PB7);        // deselect
      break;
    default:
      pinMode(cs_pin, OUTPUT);
      digitalWrite(cs_pin, HIGH);
      break;
  }

    // --- Switch pin setup ---
    pinMode(sw_pin, OUTPUT);
    digitalWrite(sw_pin, LOW); // channel disabled by default

    // --- Shunt calibration ---
    float shuntCalFloat = 819.2 * 1000000 * (max_current / pow(2, 15)) * shunt_resistor;
    uint16_t shuntCalValue = (uint16_t)shuntCalFloat;
    write_register(0x02, shuntCalValue);
}

void Channel::request_trigger(){
    /*
        In continuous mode, the device constantly samples bus voltage,
        shunt voltage, and temperature, updating current and power registers continuously.
        The current line of thought is that reading these registers while an
        internal update is in progress can result in invalid (dummy) values due
        to a timing mismatch.

        To prevent this, the device is operated in triggered mode. Writing 0x07
        to the mode field (bits 15–12) of the adc_config register (0x01) starts a conversion.
        Once the conversion completes, the CNVRF flag (bit 1 of register 0x0B) is set and is
        automatically cleared upon read, ensuring registers are only accessed after valid data is available.
    */

    uint16_t adc_config = read_register(0x01);
    uint16_t data = (adc_config & 0x0FFF) | (0x7000);
    // Serial.print("ADC config: ");
    // Serial.println(data, BIN);
    write_register(0x01, data);



}

bool Channel::conversion_done(){
    uint16_t diag_alert = read_register(0x0b);
    // Serial.print("Diag alert: ");
    // Serial.println(diag_alert, BIN);
    if((diag_alert & 0x02) == 0x02){
        return true;
    }
    return false;
}


void Channel::enable() {
    state = true;
    digitalWrite(sw_pin, HIGH);
}

void Channel::disable() {
    state = false;
    digitalWrite(sw_pin, LOW);
}

bool Channel::get_state() {
    return state;
}

// --- SPI read/write functions ---
uint16_t Channel::read_register(uint8_t reg)
{
  uint16_t value = 0;
  uint8_t addr = (reg << 2) | 0x01;

  // CS low
  if (cs_pin == PdbConfig::ChannelCsPb6) PORTB &= ~_BV(PB6);
  else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB &= ~_BV(PB7);
  else digitalWrite(cs_pin, LOW);

  SPI.beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE1));
  SPI.transfer(addr);
  value = static_cast<uint16_t>(SPI.transfer(0x00)) << 8u;
  value |= SPI.transfer(0x00);
  SPI.endTransaction();

  // CS high
  if (cs_pin == PdbConfig::ChannelCsPb6) PORTB |= _BV(PB6);
  else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB |= _BV(PB7);
  else digitalWrite(cs_pin, HIGH);

  return value;
}

void Channel::write_register(uint8_t reg, uint16_t value)
{
    uint8_t addr = (reg << 2) & 0xFE;

    // CS low
    if (cs_pin == PdbConfig::ChannelCsPb6) PORTB &= ~_BV(PB6);
    else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB &= ~_BV(PB7);
    else digitalWrite(cs_pin, LOW);

    delayMicroseconds(1);
    SPI.beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE1));
    SPI.transfer(addr);
    SPI.transfer((value >> 8) & 0xFF);
    SPI.transfer(value & 0xFF);
    SPI.endTransaction();

    // CS high
    if (cs_pin == PdbConfig::ChannelCsPb6) PORTB |= _BV(PB6);
    else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB |= _BV(PB7);
    else digitalWrite(cs_pin, HIGH);
}

// --- Measurement functions ---
int16_t Channel::get_current()
{
    int16_t rawCurrent = read_register(0x07);
    return rawCurrent;
    // return rawCurrent * (maxCurrent / pow(2, 15));
}

uint16_t Channel::get_bus_voltage()
{
    uint16_t rawBusVoltage = read_register(0x05);
    return rawBusVoltage;
    // return rawBusVoltage * 0.003125;
}

int16_t Channel::get_die_temperature()
{
    int16_t rawTemperature = read_register(0x06) >> 4;
    return rawTemperature;
    // return rawTemperature * 0.125;
}

uint32_t Channel::get_power()
{
    uint32_t value = 0;

    // CS low
    if (cs_pin == PdbConfig::ChannelCsPb6) PORTB &= ~_BV(PB6);
    else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB &= ~_BV(PB7);
    else digitalWrite(cs_pin, LOW);

    delayMicroseconds(1);
    SPI.beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE1));
    SPI.transfer((0x08 << 2) | 0x01);
    value = static_cast<uint32_t>(SPI.transfer(0x00)) << 16u;
    value |= static_cast<uint32_t>(SPI.transfer(0x00)) << 8u;
    value |= static_cast<uint32_t>(SPI.transfer(0x00));
    SPI.endTransaction();

    // CS high
    if (cs_pin == PdbConfig::ChannelCsPb6) PORTB |= _BV(PB6);
    else if (cs_pin == PdbConfig::ChannelCsPb7) PORTB |= _BV(PB7);
    else digitalWrite(cs_pin, HIGH);

    // return 0.2 * (maxCurrent / pow(2, 15)) * value;
    return value;
}

Channel::RawMeasurements Channel::read_raw_measurements()
{
    return {
        get_current(),
        get_bus_voltage(),
        get_power(),
        get_die_temperature(),
    };
}
