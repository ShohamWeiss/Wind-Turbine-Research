/*
  BME.cpp - Library wrapping bme680 class.
  Created by Shoham Weiss, February 19, 2020.
*/

#include "Arduino.h"
#include "BME.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

BME::BME()
{
  myStatus = "created";
}

void BME::start()
{
   if (!bme.begin()) {
    myStatus = "BME does not begin()";
    return;
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  myStatus = "BME begins";
}

float BME::getPressure()
{
  if (! bme.performReading()) {
        myStatus = "BME Error: Failed to perform reading in getPressure()";
        return 0.00;
      }
  myStatus = "Measured Pressure";
  return bme.pressure / 100.0;
}

float BME::getTemperature()
{
  if (! bme.performReading()) {
        myStatus = "BME Error: Failed to perform reading in getTemperature()";
        return 0.00;
      }
  myStatus = "Measured Temperature";
  return bme.temperature;
}
