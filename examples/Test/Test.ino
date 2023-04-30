
/* Forced-BMX280 Library
   soylentOrange - https://github.com/soylentOrange/Forced-BMX280

   forked from:
   Forced-BME280 Library
   Jochem van Kranenburg - jochemvk.duckdns.org - 9 March 2020
*/

#include <TinyWireM.h>      // https://github.com/adafruit/TinyWireM
#include <forcedBMX280.h>

// Create an instance of a BMX280. The parameters I2C bus and I2C address are optional. For example:
// - ForcedBMX280 climateSensor = ForcedBMX280(Wire, BMX280_I2C_ADDR);
// - ForcedBMX280 climateSensor = ForcedBMX280(TinyWireM, 0x77);


// BME280-sensor
#define BME_ADDRESS 0x76  // I2C-Address of sensor
ForcedBMX280 climateSensor = ForcedBME280(TinyWireM, BME_ADDRESS);

void setup(){
	Serial.begin(9600);
	TinyWireM.begin(); 				// Or "Wire.begin()" when not using an ATtiny.
	
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

