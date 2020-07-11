#include "Arduino.h"
#include "ANE.h"

ANE::ANE()
{
    
}

int ANE::getWindSpeed()
{
  int sensorValue = analogRead(windSensor);
  float voltage = sensorValue * (5.0 / 1023.0);
  sensorVoltage = sensorValue * voltageConversionConstant;
  if (sensorVoltage <= voltageMin) {
    windSpeed = 0; //Chech if voltage is below minimum value. if so, set wind speed to zero.
    } else {
      windSpeed = ((sensorVoltage - voltageMin) * windSpeedMax / (voltageMax - voltageMin)) * 2.232694;
      }
  x = windSpeed;
  if (x >= y) {
    y = x;
    } else {
      y = y;
      }
      
  a = sensorVoltage;
  if (a >= b) {
    b = a;
    } else {
      b = b;
      }
  if (windSpeed != prevWindSpeed) {
    prevWindSpeed = windSpeed;
    }
  
  return windSpeed;
}
