/*
 * teperatureService.h
 *
 * Definitions required for temperature reading service task
 *
 *  Created on: 17 de abr de 2018
 *      Author: guarita
 */

#ifndef TEMPERATURESERVICE_H_
#define TEMPERATURESERVICE_H_

//Temperature reading headers
#include <OneWire.h>
#include <DallasTemperature.h>

#include "parameters.h"

// temperature service task function
void temp_loop(void * pvParameters);

// function to print a device address
void printAddress(DeviceAddress deviceAddress);

// Function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress, xParams_t * pxParams);

#endif /* TEMPERATURESERVICE_H_ */
