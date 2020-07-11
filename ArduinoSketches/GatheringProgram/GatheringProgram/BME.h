/*
  BME.h - Library wrapping bme680 class.
  Created by Shoham Weiss, February 19, 2020.
*/

#ifndef BME_h
#define BME_h

#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

class BME
{
  public:
    BME();
    float getPressure();
    float getTemperature();
    void start();
    String myStatus = "";
   private:
    Adafruit_BME680 bme;
};
#endif
