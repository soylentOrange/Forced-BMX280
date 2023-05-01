# Forced-BMX280

[![arduino-library-badge](https://www.ardu-badge.com/badge/forcedBMX280.svg?)](https://www.ardu-badge.com/forcedBMX280)
[![GitHub release](https://img.shields.io/github/release/soylentOrange/Forced-BMX280.svg)](https://github.com/soylentOrange/Forced-BMX280/releases)
[![GitHub](https://img.shields.io/github/license/soylentOrange/Forced-BMX280)](https://github.com/soylentOrange/Forced-BMX280/blob/master/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/soylentOrange/Forced-BMX280)](https://github.com/soylentOrange/Forced-BMX280/issues)

---

## Abstract
Small and efficient library for reading ambient temperature and barometric pressure from Bosch Sensortec [Pressure sensor BMP280](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/) and relative humidity from Bosch Sensortec [Humidity sensor BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) with a unified interface. It was intended to be used with the [ATtiny85](https://www.microchip.com/en-us/product/ATtiny85) on the [DigiSpark](https://www.azdelivery.de/en/products/digispark-board) running the [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore) Bootloader. To reduce size of your compiled binary, different classes are available supporting only the functionality needed in your project.  

## Table of Contents

* [Why do we need this library](#Why-do-we-need-this-library)
* [Why do we need this ATtiny_PWM library](#why-do-we-need-this-ATtiny_PWM-library)
  * [Features](#features)

---

## Why do we need this library

There isn't a large amount of small, efficient and easy to use libraries for the BME280; that's what [JVKran](https://github.com/JVKran/Forced-BME280) found out.
That was the main goal of this entire library.

I needed only a temperature sensor and had a BMP280 lying around. So I modified this great library to support this sensor as well. Also, I included some checks to ensure that the sensor is connected. (...and fixed the typo in Celcius being nitpicky). I created several classes to support only the aquisitions and data format needed: 
* ForcedBMX280
  * Minimal version supporting only temperature aquisition in integer format
* ForcedBMX280Float
  * Supporting temperature aquisition in integer and float format
* ForcedBMP280
  * Supporting temperature and pressure aquisition in integer format
* ForcedBMP280Float
  * Supporting temperature and pressure aquisition in integer and float format
* ForcedBME280
  * Supporting temperature, pressure and humidity aquisition in integer format
* ForcedBME280Float
  * Supporting temperature, pressure and humidity aquisition in integer and float format

> See the full example on how to use the different classes.

The name of this respository stands for the mode in which the sensor is used. In forced mode the chip sleeps and gets waked up if data is requested. During measurements it peeks up to normal current consumption. However, when not in use, the chip only uses 0.25uA! Great for battery powered ATtiny's ;).

## Advantages of this library
- Ultra low power. In sleep mode the BME280 only uses 0.25uA!
- Ultra small footprint; this library was made with efficiency and memory usage in mind.
- Easy to use interface and functions. Despite the small size of this library, functions are still very intuitive.
- Automatically change between using Wire and TinyWire.


## Example
Examples can be obtained from within the Arduino IDE in File->Examples->forcedBMX280->[BME280_full_example/BMX280_minimal_example]. The bare minimum is given below
```c++
#include <forcedBMX280.h>

// Create an instance of a BMX280. The parameters I2C bus and I2C address are optional. 
// For example:
// - ForcedBMX280 climateSensor = ForcedBMX280(TinyWireM, BMX280_I2C_ALT_ADDR);

ForcedBMX280 climateSensor = ForcedBMX280(); 

void setup() {
	Serial.begin(9600);
	Wire.begin(); 				// Or "TinyWireM.begin()" when using an ATtiny.
	
	// you might want to check if the sensor is available when "starting it up"
	climateSensor.begin();
}

void loop() {
	Serial.print("Temperature: ");
	Serial.print(climateSensor.getTemperatureCelsius(true)/100);
	Serial.println(" °C");
	delay(2000);
}
```

> This library automatically determines wether to use TinyWireM or Wire. The only thing the user has to take care of, is to make sure begin() is called on the appropriate instance in the _setup()_; _TinyWireM.begin()_ or _Wire.begin_.

## Functions
#### uint8_t begin() 
This function initializes the library. Call before use...  
The funcion will return an error if the sensor is unavailable:
* ERROR_BUS (0x01) - Some error with the two-wire bus,
* ERROR_SENSOR_TYPE (0x02) - Chip-ID doesn't match our expectations (needs to be 0x58 for BMP280 and 0x60 for BME280),
* or 0 if everything went well.
#### uint8_t getChipID()
Will return the Chip-ID as read with begin().
#### uint8_t takeForcedMeasurement() 
This function takes a forced measurement which means getTemperatureCelsius(), getPressure() (if class ForcedBMP280 or ForcedBME280 is used) and getRelativeHumidity() (if class ForcedBME280 is used and a BME280 is connected) use the updated values. Useful in case all functions are all called at the same time or you want to check the sensor's availability.  
The funcion will return an error if the sensor is unavailable:
* ERROR_BUS (0x01) - Some error with the two-wire bus,
* or 0 if everything went well.
#### int32_t getTemperatureCelsius(const bool performMeasurement) 
This function can be used to retrieve the temperature. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getTemperatureCelsius() function is called. The function is available in all classes. 
#### float getTemperatureCelsiusAsFloat(const bool performMeasurement) 
See above, but using float for the result. The function is available in all classes ending with Float (i.e. ForcedBMX280Float, ForcedBMP280Float, ForcedBME280Float).
#### uint32_t getPressure(const bool performMeasurement) 
This function can be used to retrieve the pressure. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getPressure() function is called. The function is available in ForcedBMP280 and ForcedBME280 and their Float counterparts.
#### float getPressureAsFloat(const bool performMeasurement) 
See above, but using float for the result. The function is available in ForcedBMP280Float and ForcedBME280Float.
#### uint32_t getRelativeHumidity(const bool performMeasurement) - only when BME280 is connected
This function can be used to retrieve the humidity. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getRelativeHumidity() function is called. The function is only available in ForcedBME280 and ForcedBME280Float.
#### float getRelativeHumidityAsFloat(const bool performMeasurement) - only when BME280 is connected
See above, but using float for the result. The function is only available in ForcedBME280Float.


## Installation
#### Download
Press the green _clone or download_ button in the upper-right and download as _.ZIP_. Then go to the Arduino IDE and go to _Sketch_>Use Library->Add .ZIP Library_ and select the just downloaded zip file.

#### Library Manager
Open up the Library Manager in the Arduino IDE and search for *forcedBMX280*. Select the desired version; higher means more features ;).
