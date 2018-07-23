/*
 * temperatureService.cpp
 *
 *  Created on: 17 de abr de 2018
 *      Author: guarita
 */

#include "temperatureService.h"

#define CYCLE_RATE_MS       10000
#define MAX_COMMS_DELAY     2

void vTemperatureServiceTask(void *pvParameters) {

	// AUXILIARY VARS
	TickType_t xLastWakeTime; // store clock to guarantee periodic operation
	DeviceAddress puTempDeviceAddress; // We'll use this variable to store a found device address
	xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void * to xParams_t *

	// initialize temperature vector
	for(int i = 0; i < MAX_ONE_WIRE_DEVICES; i++) pxParams->pfTemperature[i] = -100;

	//temperature ready starts false
	pxParams->bServiceStarted = false;

	// initialize 1-wire and sensors objects
	pxParams->pxOneWire = new OneWire(ONE_WIRE_BUS);
	pxParams->pxSensors = new DallasTemperature(pxParams->pxOneWire);

	/* DS18B20 SETUP */
	//Temp Setup
	Serial.println("Dallas Temperature IC Control Library");

	// Start up the library
	Serial.println("Initializing 1-wire bus, searching devices...");
	pxParams->pxSensors->begin(); // Initialize bus and search for devices
	pxParams->pxSensors->setWaitForConversion(true); // Configure to wait temperature conversion

	// Show count of DS18 devices on the wire
	pxParams->ucNumberOfDs18Devices = 3; // pxParams->pxSensors->getDS18Count();
	Serial.print("Found ");
	Serial.print(pxParams->ucNumberOfDs18Devices, DEC);
	Serial.println(" DS18 devices.");

	// Loop through each device, print out address and set resolution.
	for (int i = 0; i < pxParams->ucNumberOfDs18Devices; i++) {

		// Search the wire for address
		if (pxParams->pxSensors->getAddress(puTempDeviceAddress, i)) {
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(puTempDeviceAddress);
			Serial.println();

			// set the resolution to TEMPERATURE_PRECISION bit
			// (Each Dallas/Maxim device is capable of several different resolutions)
			Serial.print("Setting resolution to ");
			Serial.println(TEMPERATURE_PRECISION, DEC);
			pxParams->pxSensors->setResolution(puTempDeviceAddress, TEMPERATURE_PRECISION);

			Serial.print("Resolution actually set to: ");
			Serial.print(pxParams->pxSensors->getResolution(puTempDeviceAddress), DEC);
			Serial.println();

		} else {
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(" but could not detect address. Check power and cabling");
		}
	}

	for (;;) {

		/* TEMPERATURE READING LOOP */
		vTaskDelayUntil(&xLastWakeTime, CYCLE_RATE_MS); // Set periodic

		// Call pxParams->pxSensors->requestTemperatures() to issue a global temperature request to all devices on the bus
		Serial.print("Requesting temperatures...");
		pxParams->pxSensors->requestTemperatures(); // Waits for conversion
		Serial.println("DONE");

		// Lock temperature data mutex
		xSemaphoreTake(pxParams->xTemperatureDataMutex, 50);

		// Loop through each device, update temperature vector
		// add items to display and publish queues
		for (int i = 0; i < pxParams->ucNumberOfDs18Devices; i++) {

			// Search the wire for address
			if (pxParams->pxSensors->getAddress(puTempDeviceAddress, i)) {

				// Output the device ID
				Serial.print("Temperature for device: "); Serial.println(i, DEC);
				Serial.printf("temp[%d]", i);

				// Print temperature of device
				printTemperature(puTempDeviceAddress, pxParams);
				pxParams->pfTemperature[i] = pxParams->pxSensors->getTempC(puTempDeviceAddress);

			} else {
				//else ghost device! Check your power requirements and cabling
				pxParams->pfTemperature[i] = -127; // Invalid data!

				Serial.print("Temperature for device: "); Serial.println(i, DEC);
				Serial.printf("temp[%d] ", i);	Serial.println("Invalid!");
			}
		}

		// Free temperature data mutex
		xSemaphoreGive(pxParams->xTemperatureDataMutex);

		//temperature reading tasks may use temperature
		pxParams->bServiceStarted = true;

		//  Flag temperature data ready for display Service
		xSemaphoreGive(pxParams->xTemperatureDataAvailableForDisplay);

		// Flag temperature data ready for publish Service
		xSemaphoreGive(pxParams->xTemperatureDataAvailableForPublishing);

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
	Serial.print(tempC);
	Serial.println(" C");
}
