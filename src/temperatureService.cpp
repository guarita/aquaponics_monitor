/*
 * temperatureService.cpp
 *
 *  Created on: 17 de abr de 2018
 *      Author: guarita
 */

#include "temperatureService.h"


void temp_loop(void *pvParameters) {

	// AUXILIARY VARS
	DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
	xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void* to xParams_t *
	char buff[100]; 	// buffer for string operations

	// initialize temperature vector
	for(int i = 0; i < MAX_ONE_WIRE_DEVICES; i++) pxParams->pfTemperature[i] = -100;

	//temperature ready starts false
	pxParams->bServiceStarted = false;

	// initialize mutex
	pxParams->pxOneWire = new OneWire(ONE_WIRE_BUS);
	pxParams->pxSensors = new DallasTemperature(pxParams->pxOneWire);

	/* DS18B20 SETUP */
	//Temp Setup
	Serial.println("Dallas Temperature IC Control Library");

	// Start up the library
	pxParams->pxSensors->begin();

	// Grab a count of devices on the wire
	pxParams->ucNumberOfDevices = 3;//pxParams->pxSensors->getDeviceCount();

	// locate devices on the bus
	Serial.print("Locating devices...");

	Serial.print("Found ");
	Serial.print(pxParams->pxSensors->getDeviceCount(), DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (pxParams->pxSensors->isParasitePowerMode())
		Serial.println("ON");
	else
		Serial.println("OFF");

	// Loop through each device, print out address
	for (int i = 0; i < pxParams->ucNumberOfDevices; i++) {
		// Search the wire for address
		if (pxParams->pxSensors->getAddress(tempDeviceAddress, i)) {
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(tempDeviceAddress);
			Serial.println();

			Serial.print("Setting resolution to ");
			Serial.println(TEMPERATURE_PRECISION, DEC);

			// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
			pxParams->pxSensors->setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

			Serial.print("Resolution actually set to: ");
			Serial.print(pxParams->pxSensors->getResolution(tempDeviceAddress), DEC);
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
		// call pxParams->pxSensors->requestTemperatures() to issue a global temperature request to all devices on the bus
		Serial.print("Requesting temperatures...");
		pxParams->pxSensors->requestTemperatures(); // Send the command to get temperatures

		delay(500);

		Serial.println("DONE");

		// Loop through each device, print out temperature data
		for (int i = 0; i < pxParams->ucNumberOfDevices; i++) {
			if (!pxParams->bServiceStarted)
				while (!pxParams->pxSensors->getAddress(tempDeviceAddress, i));

			// Search the wire for address
			if (pxParams->pxSensors->getAddress(tempDeviceAddress, i)) {
				// Output the device ID
				Serial.print("Temperature for device: ");
				Serial.println(i, DEC);

				Serial.printf("temp[%d]", i);

				// It responds almost immediately. Let's print out the data
				printTemperature(tempDeviceAddress, pxParams); // Use a simple function to print out the data
				pxParams->pfTemperature[i] = pxParams->pxSensors->getTempC(tempDeviceAddress);
			}
			//else ghost device! Check your power requirements and cabling
		}

		if (!(pxParams->bServiceStarted)) {
			delay(1000);
			for (int i = 0; i < pxParams->ucNumberOfDevices; i++) {
				if (pxParams->pxSensors->getAddress(tempDeviceAddress, i)) {
					memcpy(pxParams->pucTemperatureAddress[i], tempDeviceAddress, 8);

					Serial.printf("Temp Address[%02d]: ", i);
					for (int j = 0; j < 8; j++) {
						Serial.printf("%02X", pxParams->pucTemperatureAddress[i][j]);
					}
					Serial.print("\r\n");
				}

				delay(500);
			}
		}

		//temperature reading tasks may use temperature
		pxParams->bServiceStarted = true;

		delay(500);
	}

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
	for (uint8_t i = 0; i < 8; i++) {
		if (deviceAddress[i] < 16)
			Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress, xParams_t * pxParams) {
	float tempC = pxParams->pxSensors->getTempC(deviceAddress);
	Serial.print("Temp C: ");
	Serial.println(tempC);
}
