/* Forced-BMX280 Library
   soylentOrange - https://github.com/soylentOrange/Forced-BMX280

   forked from:
   Forced-BME280 Library
   Jochem van Kranenburg - jochemvk.duckdns.org - 9 March 2020
*/

#ifndef __FORCED_BMX280_HPP
#define __FORCED_BMX280_HPP

#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__) | defined(__AVR_AT90Tiny26__) | defined(__AVR_ATtiny26__)
#define FORCED_BMX280_USE_TINY_WIRE_M
#define FORCED_BMX280_USE_INTEGER_RESULTS
#endif

#include <Arduino.h>

#ifdef FORCED_BMX280_USE_TINY_WIRE_M
#include <TinyWireM.h>
#else
#include <Wire.h>
#endif

#define CHIP_ID_BMP280 (0x58)       // chip ID of BMP280
#define CHIP_ID_BME280 (0x60)       // chip ID of BME280
#define ERROR_OK (0x00)             // everything is fine
#define ERROR_BUS (0x01)            // some error with the two-wire bus
#define ERROR_SENSOR_TYPE (0x02)    // chip-ID doesn't match our expectations
#define RESET_KEY (0xB6)            // Reset value for reset register
#define STATUS_IM_UPDATE (0)        // im_update bit in status register
#define BMX280_I2C_ADDR (0x76)      // standard I2C-Address of sensor
#define BMX280_I2C_ALT_ADDR (0x77)  // alternative I2C-Address of sensor


class ForcedBMX280 {
private:
#ifdef FORCED_BMX280_USE_TINY_WIRE_M
  USI_TWI& _bus;
#else
  TwoWire& _bus;
#endif
  uint8_t _address;
  uint8_t _chipID;

  // calibration data
  int16_t _temperature[4];
  int16_t _pressure[10];
  int16_t _humidity[7];

  // fine temperature as global variable
  int32_t _BMX280t_fine;

  int16_t readTwoRegisters();
  int32_t readFourRegisters();
  uint8_t read8(uint8_t reg);
  uint8_t write8(uint8_t reg, uint8_t value);
  uint8_t setReg(uint8_t reg);

  uint8_t applyOversamplingControls();
  uint8_t readCalibrationData();

  enum class registers {
    CTRL_HUM = 0xF2,
    CTRL_MEAS = 0xF4,
    FIRST_CALIB = 0x88,      // temperature and pressure calibration data
    FIRST_HUM_CALIB = 0xA1,  // first byte of humidity calibration data
    SCND_HUM_CALIB = 0xE1,   // second part of humidity calibration data
    TEMP_MSB = 0xFA,
    HUM_MSB = 0xFD,
    PRESS_MSB = 0xF7,
    STATUS = 0xF3,
    CHIPID = 0xD0,  // Chip ID register
    RESET = 0xE0    // Reset register
  };

public:
#ifdef FORCED_BMX280_USE_TINY_WIRE_M
  ForcedBMX280(USI_TWI& bus = TinyWireM, const uint8_t address = BMX280_I2C_ADDR);
#else
  ForcedBMX280(TwoWire& bus = Wire, const uint8_t address = BMX280_I2C_ADDR);
#endif

  uint8_t begin();
  uint8_t takeForcedMeasurement();
  uint8_t getChipID();

#ifdef FORCED_BMX280_USE_INTEGER_RESULTS
  int32_t getTemperatureCelsius(const bool performMeasurement = false);
#else
  float getTemperatureCelsius(const bool performMeasurement = false);
#endif

#ifdef FORCED_BMX280_USE_INTEGER_RESULTS
  uint32_t getPressure(const bool performMeasurement = false);
#else
  float getPressure(const bool performMeasurement = false);
#endif

#ifdef FORCED_BMX280_USE_INTEGER_RESULTS
  uint32_t getRelativeHumidity(const bool performMeasurement = false);
#else
  float getRelativeHumidity(const bool performMeasurement = false);
#endif
};


#endif  //__FORCED_BMX280_HPP
