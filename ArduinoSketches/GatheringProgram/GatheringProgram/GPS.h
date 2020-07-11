/*
  GPS.h - Library wrapping GPS class.
  Created by Shoham Weiss, February 19, 2020.
*/

#ifndef GPS_h
#define GPS_h

#include "Arduino.h"
#include <Adafruit_GPS.h>

// Hardware serial port
#define GPSSerial Serial2

class GPS
{
  public:
    GPS();
    void start();
    String getLocation(bool full);
    String getTime();
    String getDate();
    String myStatus = "_";
  private:
};
#endif
