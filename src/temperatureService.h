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
#define TEMPERATURE_PRECISION 9 // Lower resolution
#define MAX_ONE_WIRE_DEVICES 20
typedef void (* pfuncTaskFunctionPtr)(void *);

class TemperatureReaderService{

  public:
    TemperatureReaderService();

    // loop function for task
    void loop(void * pvParameters);

    // function to print a device address
    void printAddress(DeviceAddress deviceAddress);

    // Function to print the temperature for a device
    void printTemperature(DeviceAddress deviceAddress);

    // GETTERS
    bool getServiceStarted();
    void getTemperature(float * pfTemperature);
    void getTemperatureAddress(uint8_t * pucTemperatureAddress);
    uint8_t getNumberOfDevices();

  private:

    bool vLockOutputVars();

    bool vReleaseOutputVars();

    xSemaphoreHandle xServiceMutex;

    // OUTPUT VARS
    // flag for available teperature information
    bool serviceStarted;

    // temperatures vector
    float temperature[MAX_ONE_WIRE_DEVICES];

    // temperature device addresses
    uint8_t temperatureAddress[MAX_ONE_WIRE_DEVICES][8];

    // temperature sensors addresses
    uint8_t ucNumberOfDevices;

    // AUXILIARY VARS
    // We'll use this variable to store a found device address
    DeviceAddress tempDeviceAddress;

    // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
    OneWire * oneWire;

    // Pass our oneWire reference to Dallas Temperature.
    DallasTemperature * sensors;

    // buffer for string operations
    char buff[100];

};

#endif /* TEMPERATURESERVICE_H_ */
