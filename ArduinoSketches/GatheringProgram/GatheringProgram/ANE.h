#ifndef ANE_h
#define ANE_h

#include "Arduino.h"

class  ANE
{
  public:
    ANE();
    int getWindSpeed();
    double x = 0;
    double y = 0;
    double a = 0;
    double b = 0;
    int windSensor = A15;
    float voltageMax = 2.0;
    float voltageMin = 0.4;
    float voltageConversionConstant = 0.004882814;
    float sensorVoltage = 0;
    float windSpeedMin = 0;
    float windSpeedMax = 32;
    int windSpeed = 0;
    int prevWindSpeed = 0;

};

#endif
