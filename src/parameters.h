#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <OneWire.h>
#include <DallasTemperature.h>

/* Global definitions go here */

//Temperature Service Definitions */
#define ONE_WIRE_BUS 22 // Data wire is plugged into GPIO 22 on the ESP32
#define TEMPERATURE_PRECISION 9 // Lower resolution
#define MAX_ONE_WIRE_DEVICES 20

/* Gloal parameters must be put here */
typedef struct params{

  /* Temperature Service Vars */
  // Number of temperature reading devices found/considered
  uint8_t ucNumberOfDs18Devices;

  // flag indicating temperature service ready
  bool bTempStarted;

  // temperatures vector
  float pfTemperature[MAX_ONE_WIRE_DEVICES];

  // flag for available teperature information
  bool bServiceStarted;

  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  OneWire * pxOneWire;

  // Pass our oneWire reference to Dallas Temperature.
  DallasTemperature * pxSensors;

  // temperature device addresses
  uint8_t pucTemperatureAddress[MAX_ONE_WIRE_DEVICES][8];

  /* Display Service Vars */
  SemaphoreHandle_t xTemperatureDataAvailableForPublishing;
  SemaphoreHandle_t xTemperatureDataMutex;
  SemaphoreHandle_t xTemperatureDataAvailableForDisplay;

} xParams_t;

#endif
