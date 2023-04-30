/// @file

/* Forced-BMX280 Library
   soylentOrange - https://github.com/soylentOrange/Forced-BMX280

   forked from:
   Forced-BME280 Library
   Jochem van Kranenburg - jochemvk.duckdns.org - 9 March 2020
*/

#include "forcedBMX280.h"

/* ForcedBMX280
   Basic version of the sensor.
   Should give you minimum size after compliation.

   Supports temperature aquisition in integer format only
*/

/// \brief
/// Constructor of ForcedBMX280
/// \details
/// This creates a ForcedBMX280 object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBMX280::ForcedBMX280(USI_TWI& bus, const uint8_t address)
  :
#else
ForcedBMX280::ForcedBMX280(TwoWire& bus, const uint8_t address)
  :
#endif
    _bus(bus),
    _address(address)  {
  /*..*/
}

/// \brief
/// Read 16 bits
/// \details
/// This function reads 16 bits from the I2C bus.
int16_t ForcedBMX280::readTwoRegisters() {
  uint8_t lo = _bus.read();
  uint8_t hi = _bus.read();
  return hi << 8 | lo;
}

/// \brief
/// Read 32 bits
/// \details
/// This function reads 32 bits from the I2C bus.
int32_t ForcedBMX280::readFourRegisters() {
  uint8_t msb = _bus.read();
  uint8_t lsb = _bus.read();
  uint8_t xlsb = _bus.read();
  return (uint32_t)msb << 12 | (uint32_t)lsb << 4 | (xlsb >> 4 & 0x0F);
}

/// \brief
/// Read 8 bits
/// \details
/// This function reads 8 bits from the register (reg) via I2C bus.
uint8_t ForcedBMX280::read8(uint8_t reg) {
  _bus.beginTransmission(_address);
  _bus.write(reg);
  _bus.endTransmission();
  _bus.requestFrom(_address, (byte)1);
  return _bus.read();
}

/// \brief
/// Write 8 bits
/// \details
/// This function writes 8 bits (value) to the device into to register (reg).
/// Returns an error code if there was one from the bus
uint8_t ForcedBMX280::write8(uint8_t reg, uint8_t value) {
  _bus.beginTransmission(_address);
  _bus.write(reg);
  _bus.write(value);
  if (_bus.endTransmission()) return ERROR_BUS;

  return ERROR_OK;
}

/// \brief
/// Set register
/// \details
/// This function selects a register (reg) of the sensor.
/// Returns an error code if there was one from the bus
uint8_t ForcedBMX280::setReg(uint8_t reg) {
  _bus.beginTransmission(_address);
  _bus.write(reg);
  if (_bus.endTransmission()) return ERROR_BUS;

  return ERROR_OK;
}


/// \brief
/// begin
/// \details
/// This applies the set oversampling controls and reads the calibration
/// data from the register. 
/// Returns an error code if there was one from the bus the chipID is not matching BME280 or BMP280
uint8_t ForcedBMX280::begin() {
  _bus.begin();

  // Check that something is attached to the bus at the given address
  _bus.beginTransmission(_address);
  if (_bus.endTransmission()) return ERROR_BUS;

  // Read chip ID
  _chipID = read8((uint8_t)registers::CHIPID);

  // Check sensor ID BMP280 or BME280
  if ((_chipID != CHIP_ID_BMP280) && ((_chipID != CHIP_ID_BME280))) {
    // BMP280 / BME280 not found
    return ERROR_SENSOR_TYPE;
  }

  // Generate soft-reset
  if (write8((uint8_t)registers::RESET, (uint8_t)RESET_KEY)) return ERROR_BUS;

  // Wait for copy completion NVM data to image registers
  delay(10);
  while ((read8((uint8_t)registers::STATUS) & (_BV(STATUS_IM_UPDATE)))) {
    delay(10);
  }

  // set mode of sensor
  if (applyOversamplingControls()) return ERROR_BUS;

  // read factory trimming parameters
  if (readCalibrationData()) return ERROR_BUS;

  return ERROR_OK;
}

/// \brief
/// getChipID
/// \details
/// Chip ID as read with begin()
uint8_t ForcedBMX280::getChipID() {
  // Return chip ID
  return _chipID;
}

/// \brief
/// Take forced measurement
/// \details
/// This function takes a forced measurement. That is, the BME280 is woken up to take
/// a measurement after which it goes back to sleep. During this sleep, it consumes
/// 0.25uA!
/// Returns an error code if there was one from the bus
uint8_t ForcedBMX280::takeForcedMeasurement() {

  // ctrl_meas - see datasheet section 5.4.5
  // forced mode: ctrl_meas[0..1] 0b01 (0b11 for normal and 0b00 for sleep mode)
  // pressure oversampling x 1: ctrl_meas[4..2] 0b001
  // temperature oversampling x 1: ctrl_meas[7..5] 0b001
  if (write8((uint8_t)registers::CTRL_MEAS, 0b00100101)) return ERROR_BUS;
  return ERROR_OK;
}

/// \brief
/// Apply oversampling controls
/// \details
/// This function sets the sampling controls to values that are
/// suitable for all kinds of applications.
/// Returns an error code if there was one from the bus
uint8_t ForcedBMX280::applyOversamplingControls() {

  // Set in sleep mode to provide write access to the “config” register
  if (write8((uint8_t)registers::CTRL_MEAS, 0)) return ERROR_BUS;

  // ctrl_meas - see datasheet section 5.4.5
  // forced mode: ctrl_meas[0..1] 0b01 (0b11 for normal and 0b00 for sleep mode)
  // pressure oversampling x 1: ctrl_meas[4..2] 0b001
  // temperature oversampling x 1: ctrl_meas[7..5] 0b001
  if (write8((uint8_t)registers::CTRL_MEAS, 0b00100101)) return ERROR_BUS;
  return ERROR_OK;
}

/// \brief
/// Read Calibrations
/// \details
/// This functions reads the calibration data after which it is
/// stored in the temperature, pressure and humidity arrays for
/// later use in compensation.
/// Returns an error code if there was one from the bus
uint8_t ForcedBMX280::readCalibrationData() {
  if (setReg((uint8_t)registers::TEMP_CALIB)) return ERROR_BUS;
  _bus.requestFrom(_address, (uint8_t)6);
  // read 6 bytes for temperature calibration data
  for (int i = 1; i <= 3; i++) _temperature[i] = readTwoRegisters();  // Temperature

  // get temperature reading to initialize BMX280t_fine
  if (takeForcedMeasurement()) return ERROR_BUS;
  getTemperatureCelsius();

  // done
  return ERROR_OK;
}


/// \brief
/// Get Temperature Celsius
/// \details
/// This function retrieves the compensated temperature
int32_t ForcedBMX280::getTemperatureCelsius(const bool performMeasurement) {
  _bus.beginTransmission(_address);
  if (performMeasurement) {
    _bus.write((uint8_t)registers::CTRL_MEAS);
    _bus.write(0b00100101);
  }
  _bus.write((uint8_t)registers::TEMP_MSB);
  _bus.endTransmission();
  _bus.requestFrom(_address, (uint8_t)3);
  int32_t adc = readFourRegisters();
  int32_t var1 = ((((adc >> 3) - ((int32_t)((uint16_t)_temperature[1]) << 1))) * ((int32_t)_temperature[2])) >> 11;
  int32_t var2 = ((((adc >> 4) - ((int32_t)((uint16_t)_temperature[1]))) * ((adc >> 4) - ((int32_t)((uint16_t)_temperature[1])))) >> 12);
  var2 = (var2 * ((int32_t)_temperature[3])) >> 14;
  _BMX280t_fine = var1 + var2;
  int32_t temperature = (_BMX280t_fine * 5 + 128) >> 8;

  return temperature;
}


/* ForcedBMX280_float
   Enhanced version of the sensor.

   Supports temperature aquisition in integer and float format
*/
/// \brief
/// Constructor
/// \details
/// This creates an object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBMX280_float::ForcedBMX280_float(USI_TWI& bus, const uint8_t address)
  :
#else
ForcedBMX280_float::ForcedBMX280_float(TwoWire& bus, const uint8_t address)
  :
#endif
    _bus(bus),
    _address(address) {
  /*..*/
}

/// \brief
/// Get Temperature Celsius
/// \details
/// This function retrieves the compensated temperature
float ForcedBMX280_float::getTemperatureCelsius_float(const bool performMeasurement) {
  return float(getTemperatureCelsius(performMeasurement) / 100.0);
}



/* ForcedBMP280
   Version of the sensor supporting temperature and pressure aquisition.

   Supports temperature and pressure aquisition in integer format only
*/
/// \brief
/// Constructor
/// \details
/// This creates an object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBMP280::ForcedBMP280(USI_TWI& bus, const uint8_t address)
  :
#else
ForcedBMP280::ForcedBMP280(TwoWire& bus, const uint8_t address)
  :
#endif
    _bus(bus),
    _address(address) {
  /*..*/
}

/// \brief
/// Read Calibrations
/// \details
/// This functions reads the calibration data after which it is
/// stored in the temperature, pressure and humidity arrays for
/// later use in compensation.
/// Returns an error code if there was one from the bus
uint8_t ForcedBMP280::readCalibrationData() {
  if (setReg((uint8_t)registers::TEMP_CALIB)) return ERROR_BUS;
  _bus.requestFrom(_address, (uint8_t)24);
  // read 24 bytes for temperature and pressure calibration data
  for (int i = 1; i <= 3; i++) _temperature[i] = readTwoRegisters();  // Temperature
  for (int i = 1; i <= 9; i++) _pressure[i] = readTwoRegisters();     // Pressure

  // get temperature reading to initialize _BMX280t_fine
  if (takeForcedMeasurement()) return ERROR_BUS;
  getTemperatureCelsius();

  // done
  return ERROR_OK;
}


/// \brief
/// Get Pressure
/// \details
/// This function retrieves the compensated pressure
uint32_t ForcedBMP280::getPressure(const bool performMeasurement) {
  _bus.beginTransmission(_address);
  if (performMeasurement) {
    _bus.write((uint8_t)registers::CTRL_MEAS);
    _bus.write(0b00100101);
  }
  _bus.write((uint8_t)registers::PRESS_MSB);
  _bus.endTransmission();
  _bus.requestFrom(_address, (uint8_t)3);

  int32_t adc = readFourRegisters();
  int32_t var1 = (((int32_t)_BMX280t_fine) >> 1) - (int32_t)64000;
  int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)_pressure[6]);
  var2 = var2 + ((var1 * ((int32_t)_pressure[5])) << 1);
  var2 = (var2 >> 2) + (((int32_t)_pressure[4]) << 16);
  var1 = (((_pressure[3] * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)_pressure[2]) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((int32_t)((uint16_t)_pressure[1]))) >> 15);

  if (var1 == 0) {
    return 0;
  }
  uint32_t p = (((uint32_t)(((int32_t)1048576) - adc) - (var2 >> 12))) * 3125;
  if (p < 0x80000000) {
    p = (p << 1) / ((uint32_t)var1);
  } else {
    p = (p / (uint32_t)var1) * 2;
  }

  var1 = (((int32_t)_pressure[9]) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
  var2 = (((int32_t)(p >> 2)) * ((int32_t)_pressure[8])) >> 13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + _pressure[7]) >> 4));

  return p;
}


/* ForcedBMP280_float
   Version of the sensor supporting temperature and pressure aquisition.

   Supports temperature and pressure aquisition in integer and float format
*/
/// \brief
/// Constructor
/// \details
/// This creates an object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBMP280_float::ForcedBMP280_float(USI_TWI& bus, const uint8_t address)
:
#else
ForcedBMP280_float::ForcedBMP280_float(TwoWire& bus, const uint8_t address)
:
#endif
    _bus(bus),
    _address(address) {
  /*..*/
}

/// \brief
/// Get Temperature Celsius
/// \details
/// This function retrieves the compensated temperature
float ForcedBMP280_float::getTemperatureCelsius_float(const bool performMeasurement) {
  return float(getTemperatureCelsius(performMeasurement) / 100.0);
}


/// \brief
/// Get Pressure
/// \details
/// This function retrieves the compensated pressure 
float ForcedBMP280_float::getPressure_float(const bool performMeasurement) {
  return float(getPressure(performMeasurement) / 100.0);
}

/* ForcedBME280
   Version of the sensor supporting temperature, pressure and humidity aquisition.

   Supports temperature, pressure and humidity aquisition in integer format
*/
/// \brief
/// Constructor
/// \details
/// This creates an object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBME280::ForcedBME280(USI_TWI& bus, const uint8_t address)
:
#else
ForcedBME280::ForcedBME280(TwoWire& bus, const uint8_t address)
:
#endif
    _bus(bus),
    _address(address) {
  /*..*/
}

/// \brief
/// Apply oversampling controls
/// \details
/// This function sets the sampling controls to values that are
/// suitable for all kinds of applications.
/// Returns an error code if there was one from the bus
uint8_t ForcedBME280::applyOversamplingControls() {

  // Set in sleep mode to provide write access to the “config” register
  if (write8((uint8_t)registers::CTRL_MEAS, 0)) return ERROR_BUS;

  // humidity oversampling - see datasheet section 5.4.3
  // oversampling x 1: 0x01
  // only to be set when a BME280 is used
  if (_chipID == CHIP_ID_BME280) {
    if (write8((uint8_t)registers::CTRL_HUM, 0b00000001)) return ERROR_BUS;
  }

  // ctrl_meas - see datasheet section 5.4.5
  // forced mode: ctrl_meas[0..1] 0b01 (0b11 for normal and 0b00 for sleep mode)
  // pressure oversampling x 1: ctrl_meas[4..2] 0b001
  // temperature oversampling x 1: ctrl_meas[7..5] 0b001
  if (write8((uint8_t)registers::CTRL_MEAS, 0b00100101)) return ERROR_BUS;
  return ERROR_OK;
}

/// \brief
/// Read Calibrations
/// \details
/// This functions reads the calibration data after which it is
/// stored in the temperature, pressure and humidity arrays for
/// later use in compensation.
/// Returns an error code if there was one from the bus
uint8_t ForcedBME280::readCalibrationData() {
  if (setReg((uint8_t)registers::TEMP_CALIB)) return ERROR_BUS;
  _bus.requestFrom(_address, (uint8_t)24);
  // read 24 bytes for temperature and pressure calibration data
  for (int i = 1; i <= 3; i++) _temperature[i] = readTwoRegisters();  // Temperature
  for (int i = 1; i <= 9; i++) _pressure[i] = readTwoRegisters();     // Pressure

  // read humidity calibration data in case its really a BME280
  if (_chipID == CHIP_ID_BME280) {
    // read 1. byte of humidity calibration data
    _humidity[1] = read8((uint8_t)registers::FIRST_HUM_CALIB);

    // read second part of humidity calibration data
    if (setReg((uint8_t)registers::SCND_HUM_CALIB)) return ERROR_BUS;
    _bus.requestFrom(_address, (uint8_t)7);
    _humidity[2] = readTwoRegisters();
    _humidity[3] = (uint8_t)_bus.read();
    uint8_t e4 = _bus.read();
    uint8_t e5 = _bus.read();
    _humidity[4] = ((int16_t)((e4 << 4) + (e5 & 0x0F)));
    _humidity[5] = ((int16_t)((_bus.read() << 4) + ((e5 >> 4) & 0x0F)));
    _humidity[6] = ((int8_t)_bus.read());
  }

  // get temperature reading to initialize BMX280t_fine
  if (takeForcedMeasurement()) return ERROR_BUS;
  getTemperatureCelsius();

  // done
  return ERROR_OK;
}


/// \brief
/// Get Humidity
/// \details
/// This function retrieves the compensated humidity 
uint32_t ForcedBME280::getRelativeHumidity(const bool performMeasurement) {
  // silently bail out if it is the wrong type of sensor
  if (_chipID != CHIP_ID_BME280) return 0;

  _bus.beginTransmission(_address);
  if (performMeasurement) {
    _bus.write((uint8_t)registers::CTRL_MEAS);
    _bus.write(0b00100101);
  }
  _bus.write((uint8_t)registers::HUM_MSB);
  _bus.endTransmission();
  _bus.requestFrom(_address, (uint8_t)2);
  uint8_t hi = _bus.read();
  uint8_t lo = _bus.read();
  int32_t adc = (uint16_t)(hi << 8 | lo);
  int32_t var1;
  var1 = (_BMX280t_fine - ((int32_t)76800));
  var1 = (((((adc << 14) - (((int32_t)_humidity[4]) << 20) - (((int32_t)_humidity[5]) * var1)) + ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)_humidity[6])) >> 10) * (((var1 * ((int32_t)_humidity[3])) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)_humidity[2]) + 8192) >> 14));
  var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)_humidity[1])) >> 4));
  var1 = (var1 < 0 ? 0 : var1);
  var1 = (var1 > 419430400 ? 419430400 : var1);
  uint32_t humidity = (uint32_t)((var1 >> 12) * 25) >> 8;
  return humidity;
}

/* ForcedBME280_float
   Version of the sensor supporting temperature, pressure and humidity aquisition.

   Supports temperature, pressure and humidity aquisition in integer and float format
*/
/// \brief
/// Constructor
/// \details
/// This creates an object from the mandatory TwoWire-bus
/// and the address of the chip to communicate with.
#ifdef FORCED_BMX280_ATTINY
ForcedBME280_float::ForcedBME280_float(USI_TWI& bus, const uint8_t address)
:
#else
ForcedBME280_float::ForcedBME280_float(TwoWire& bus, const uint8_t address)
:
#endif
    _bus(bus),
    _address(address) {
  /*..*/
}

/// \brief
/// Get Temperature Celsius
/// \details
/// This function retrieves the compensated temperature
float ForcedBME280_float::getTemperatureCelsius_float(const bool performMeasurement) {
  return float(getTemperatureCelsius(performMeasurement) / 100.0);
}

/// \brief
/// Get Pressure
/// \details
/// This function retrieves the compensated pressure 
float ForcedBME280_float::getPressure_float(const bool performMeasurement) {
  return float(getPressure(performMeasurement) / 100.0);
}

/// \brief
/// Get Humidity
/// \details
/// This function retrieves the compensated humidity as described
/// on page 50 of the BME280 Datasheet.
float ForcedBME280_float::getRelativeHumidity_float(const bool performMeasurement) {
  return float(getRelativeHumidity(performMeasurement) / 100.0);
}
