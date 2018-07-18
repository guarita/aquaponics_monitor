#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "parameters.h"

// Wifi client (to access RSSI data)
#include <WiFi.h>


//OLED headers
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
// temperature display task function
void vDisplayServiceTask(void *params);

#endif
