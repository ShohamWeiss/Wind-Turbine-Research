/*
  GPS.cpp - Library wrapping gps class.
  Created by Shoham Weiss, February 13, 2020.
*/

#include "Arduino.h"
#include "GPS.h"
#include <Adafruit_GPS.h>

Adafruit_GPS gps(&GPSSerial);

GPS::GPS()
{
  GPSSerial.begin(9600);
  myStatus = "created";
}

void GPS::start()
{
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  gps.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  myStatus = "started";
}

String GPS::getLocation(bool full)
{
  String message = "";

  long unsigned startTime = millis();
  while(!gps.available())
  {
    if (millis() - startTime > 1000)
    {
      message = "UNAVAILABLE";
      return message;
    }
  }
  
  while(!gps.newNMEAreceived())
  {
    char c = gps.read();
  }
//  Serial.println(gps.lastNMEA()); // this also sets the newNMEAreceived() flag to false
  if (!gps.parse(gps.lastNMEA())) // this also sets the newNMEAreceived() flag to falsen
  {
    return message; // we can fail to parse a sentence in which case we should just wait for another  
  }
  message = String(gps.latitude, 4) + gps.lat + ", " + String(gps.longitude,4) + gps.lon;
  if (!full)
  {
    message = String(gps.latitude, 4).substring(3,9) + gps.lat + ", " + String(gps.longitude,4).substring(3,9) + gps.lon;
  }
    
  Serial.println("Getting Location From GPS: " + message);
  
  return message;
}

String GPS::getTime()
{
  String message = "NONE";
  while (!gps.available());
  
  while(!gps.newNMEAreceived())
  {
    char c = gps.read();
  }
//  Serial.println(gps.lastNMEA()); // this also sets the newNMEAreceived() flag to false
  if (!gps.parse(gps.lastNMEA())) // this also sets the newNMEAreceived() flag to false
  {
    return getTime(); // we can fail to parse a sentence in which case we should just wait for another  
  }
  message = (String)gps.hour + "_" + (String)gps.minute + "_" + (String)gps.seconds;
    
  Serial.println("Reading Time from GPS: " + message);
  
  return message;
}

String GPS::getDate()
{
  String message = "NONE";
  while (!gps.available());
  
  while(!gps.newNMEAreceived())
  {
    char c = gps.read();
  }
//  Serial.println(gps.lastNMEA()); // this also sets the newNMEAreceived() flag to false
  if (!gps.parse(gps.lastNMEA())) // this also sets the newNMEAreceived() flag to false
  {
    return getDate(); // we can fail to parse a sentence in which case we should just wait for another  
  }
  message = (String)gps.month + "_" + (String)gps.day + "_" + (String)gps.year + "_" + (String)gps.hour + "_" + (String)gps.minute + "_" + (String)gps.seconds;
    
  Serial.println("Reading Datetime from GPS: " + message);
  
  return message;
}
