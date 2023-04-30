# Forced-BMX280
There isn't a large amount of small, efficient and easy to use libraries for the BME280; that's what [JVKran](https://github.com/JVKran/Forced-BME280) found out.
That's the main goal of this entire library.

I needed only a temperature sensor and had a BMP280 lying around. So I modified this great library to support this sensor as well. Also, I included some checks to ensure that the sensor is connected. (...and fixed the typo in Celcius being nitpicky)

The name of this respository stands for the mode in which the sensor is used. In forced mode the chip sleeps and gets waked up if data is requested. During measurements it peeks up to normal current consumption. However, when not in use, the chip only uses 0.25uA! Great for battery powered ATtiny's ;).

## Advantages of this library
- Ultra low power. In sleep mode the BME280 only uses 0.25uA!
- Ultra small footprint; this library was made with efficiency and memory usage in mind.
- Easy to use interface and functions. Despite the small size of this library, functions are still very intuitive.
- Automatically change between using Wire and TinyWire.


## Example
There has been given an example that can be obtained from below or from within the Arduino IDE in Examples->Forced-BMX280->Test.
```c++
#include <forcedBMX280.h>

// Create an instance of a BMX280. The parameters I2C bus and I2C address are optional. For example:
// if a BMP280 is connected or you don't care about humidity:
// - ForcedBMX280 climateSensor = ForcedBMP280(Wire, 0x76);
// - ForcedBMX280 climateSensor = ForcedBMP280(TinyWireM, 0x77);

// if a BME280 is connected; will allow humidity measurement
// - ForcedBMX280 climateSensor = ForcedBME280(Wire, 0x76);

ForcedBMX280 climateSensor = ForcedBME280(); 

void setup(){
	Serial.begin(9600);
	Wire.begin(); 				// Or "TinyWireM.begin()" when using an ATtiny.
	
	// you might want to check if the sensor is available when "starting it up"
	while(climateSensor.begin()) {
	  delay(100);
	} 
}

void loop(){
	climateSensor.takeForcedMeasurement();
	Serial.print("Temperature: ");
	Serial.print(climateSensor.getTemperatureCelsius());
	Serial.print(", Humidity: ");
	Serial.print(climateSensor.getRelativeHumidity());
	Serial.print(" and Pressure: ");
	Serial.print(climateSensor.getPressure());
	Serial.println();
	delay(1000);
	// Perform measurement integrated getTemperature(); useful for when only one value has to be used.
	Serial.print(climateSensor.getTemperatureCelsius(true));
	Serial.println();
	delay(5000);
}
```

> This library automatically determines wether to use TinyWireM or Wire. The only thing the user has to take care of, is to make sure begin() is called
on the appropriate instance in the _setup()_; _TinyWireM.begin()_ or _Wire.begin_.

## Functions
#### takeForcedMeasurement() 
This function takes a forced measurement which means getTemperatureCelcius(), getPressure() (and getRelativeHumidity() if a BME280 is connected) use the updated values. Useful in case all functions are all called at the same time. The funcion will return an error if the sensor might be unavailable.
#### getTemperatureCelsius(const bool performMeasurement) 
This function can be used to retrieve the temperature. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getTemperatureCelcius() function is called.
#### getPressure(const bool performMeasurement) 
This function can be used to retrieve the pressure. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getPressure() function is called.
#### getRelativeHumidity(const bool performMeasurement) - only when BME280 is connected
This function can be used to retrieve the humidity. The parameter defaults to false which means takeForcedMeasurement() should be called first to make sure updated values are used. If the passed parameter is equal to true, a forced measurement is taken; useful in case only the getRelativeHumidity() function is called.


## Installation
#### Download
Press the green _clone or download_ button in the upper-right and download as _.ZIP_. Then go to the Arduino IDE and go to _Sketch_>Use Library->Add .ZIP Library_ and select the just downloaded zip file.

#### Library Manager
Open up the Library Manager in the Arduino IDE and search for *Forced-BMX280*. Select the desired version; higher means more features ;).
