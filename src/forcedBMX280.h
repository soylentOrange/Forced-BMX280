
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

class ForcedBMX280 {
	private:
		#ifdef FORCED_BMX280_USE_TINY_WIRE_M
		USI_TWI & bus;
		#else
		TwoWire & bus;	
		#endif
		uint8_t address;

		int16_t temperature[4];
		int16_t pressure[10];
		int16_t humidity[7];
		int32_t BMX280t_fine;

		int16_t readTwoRegisters();
		int32_t readFourRegisters();

		void applyOversamplingControls();
		void readCalibrationData();

		enum class registers {
			CTRL_HUM = 0xF2,
			CTRL_MEAS = 0xF4,
			FIRST_CALIB = 0x88,
			SCND_CALIB = 0xE1,
			TEMP_MSB = 0xFA,
			HUM_MSB = 0xFD,
			PRESS_MSB = 0xF7,
			STATUS = 0xF3
		};

	public:
		#ifdef FORCED_BMX280_USE_TINY_WIRE_M
		ForcedBMX280(USI_TWI & bus = TinyWireM, const uint8_t address = 0x76);
		#else
		ForcedBMX280(TwoWire & bus = Wire, const uint8_t address = 0x76);	
		#endif

		bool begin();
		bool takeForcedMeasurement();

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


#endif //__FORCED_BMX280_HPP
