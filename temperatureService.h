/*
 * teperatureService.h
 *
 * DS18B20 definitions
 *
 *  Created on: 17 de abr de 2018
 *      Author: guarita
 */
#ifndef TEMPERATURESERVICE_H_
#define TEMPERATURESERVICE_H_

//Temperature reading headers
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into GPIO 22 on the ESP32
#define ONE_WIRE_BUS 22
#define TEMPERATURE_PRECISION 12 // Lower resolution
#define MAX_ONE_WIRE_DEVICES 20

// function to print a device address
void printAddress(DeviceAddress deviceAddress);

// Temperature service task
void temp_loop(void *params);

// Function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress);

#endif /* TEMPERATURESERVICE_H_ */