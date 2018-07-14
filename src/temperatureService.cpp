/*
 * temperatureService.cpp
 *
 *  Created on: 17 de abr de 2018
 *      Author: guarita
 */

#include "temperatureService.h"

TemperatureReaderService::TemperatureReaderService(){

	// initialize temperature vector
	for(int i = 0; i < MAX_ONE_WIRE_DEVICES; i++) this->temperature[i] = -100;

	//temperature ready starts false
	this->serviceStarted = false;

	// initialize mutex
	this->xServiceMutex = xSemaphoreCreateMutex();

	this->oneWire = new OneWire(ONE_WIRE_BUS);

	this->sensors = new DallasTemperature(this->oneWire);

}

void TemperatureReaderService::loop(void *pvParameters) {

	/* DS18B20 SETUP */
	//Temp Setup
	Serial.println("Dallas Temperature IC Control Library");

	// Start up the library
	this->sensors->begin();

	// Grab a count of devices on the wire
	this->ucNumberOfDevices = 3;//this->sensors->getDeviceCount();

	// locate devices on the bus
	Serial.print("Locating devices...");

	Serial.print("Found ");
	Serial.print(this->sensors->getDeviceCount(), DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (this->sensors->isParasitePowerMode())
		Serial.println("ON");
	else
		Serial.println("OFF");

	// Loop through each device, print out address
	for (int i = 0; i < this->ucNumberOfDevices; i++) {
		// Search the wire for address
		if (this->sensors->getAddress(tempDeviceAddress, i)) {
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(tempDeviceAddress);
			Serial.println();

			Serial.print("Setting resolution to ");
			Serial.println(TEMPERATURE_PRECISION, DEC);

			// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
			this->sensors->setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

			Serial.print("Resolution actually set to: ");
			Serial.print(this->sensors->getResolution(tempDeviceAddress), DEC);
			Serial.println();
		} else {
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(
					" but could not detect address. Check power and cabling");
		}
	}

	for (;;) {

		/* TEMPERATURE READING LOOP */
		// call sensors->requestTemperatures() to issue a global temperature request to all devices on the bus
		Serial.print("Requesting temperatures...");
		this->sensors->requestTemperatures(); // Send the command to get temperatures

		delay(500);

		Serial.println("DONE");

		// lock output vars for writing
		if(!this->vLockOutputVars())
			Serial.println("Temp Service: ERROR locking vars.");

		// Loop through each device, print out temperature data
		for (int i = 0; i < this->ucNumberOfDevices; i++) {
			if (!this->serviceStarted)
				while (!this->sensors->getAddress(tempDeviceAddress, i));

			// Search the wire for address
			if (this->sensors->getAddress(tempDeviceAddress, i)) {
				// Output the device ID
				Serial.print("Temperature for device: ");
				Serial.println(i, DEC);

				Serial.printf("temp[%d]", i);

				// It responds almost immediately. Let's print out the data
				printTemperature(tempDeviceAddress); // Use a simple function to print out the data
				this->temperature[i] = this->sensors->getTempC(this->tempDeviceAddress);
			}
			//else ghost device! Check your power requirements and cabling
		}

		if (!(this->serviceStarted)) {
			delay(1000);
			for (int i = 0; i < this->ucNumberOfDevices; i++) {
				if (this->sensors->getAddress(this->tempDeviceAddress, i)) {
					memcpy(this->temperatureAddress[i], this->tempDeviceAddress, 8);

					Serial.printf("Temp Address[%02d]: ", i);
					for (int j = 0; j < 8; j++) {
						Serial.printf("%02X", this->temperatureAddress[i][j]);
					}
					Serial.print("\r\n");
				}

				delay(500);
			}
		}

		//temperature reading tasks may use temperature
		this->serviceStarted = true;

		// release output vars
		if(!this->vReleaseOutputVars())
			Serial.println("Temp Service: ERROR releasing vars.");

		delay(500);
	}

}

// function to print a device address
void TemperatureReaderService::printAddress(DeviceAddress deviceAddress) {
	for (uint8_t i = 0; i < 8; i++) {
		if (deviceAddress[i] < 16)
			Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

// function to print the temperature for a device
void TemperatureReaderService::printTemperature(DeviceAddress deviceAddress) {
	float tempC = this->sensors->getTempC(deviceAddress);
	Serial.print("Temp C: ");
	Serial.println(tempC);
//	Serial.print("Temp F: ");
//	Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

bool TemperatureReaderService::getServiceStarted(){
	bool ret;
	this->vLockOutputVars();
	ret = this->serviceStarted;
	this->vReleaseOutputVars();
	return ret;
}

void TemperatureReaderService::getTemperature(float * pfTemperature){
	this->vLockOutputVars();
	memcpy(pfTemperature, this->temperature, MAX_ONE_WIRE_DEVICES * sizeof(float));
	this->vReleaseOutputVars();
}

void TemperatureReaderService::getTemperatureAddress(uint8_t * pucTemperatureAddress){
	this->vLockOutputVars();
	memcpy(pucTemperatureAddress, this->temperatureAddress, MAX_ONE_WIRE_DEVICES * 8 * sizeof(uint8_t));
	this->vReleaseOutputVars();
}

uint8_t TemperatureReaderService::getNumberOfDevices(){
	uint8_t ret;
	this->vLockOutputVars();
	ret = this->ucNumberOfDevices;
	this->vReleaseOutputVars();
	return ret;
}

bool TemperatureReaderService::vLockOutputVars(){
	return xSemaphoreTake(this->xServiceMutex, 1000);
}

bool TemperatureReaderService::vReleaseOutputVars(){
	return xSemaphoreGive(this->xServiceMutex);
}
