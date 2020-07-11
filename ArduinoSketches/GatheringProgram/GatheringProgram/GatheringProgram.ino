/*
  DAQ.ino - Data Acquisition Program for PHYS 398.
  Created by Shoham Weiss, Spring 2020.
          by Tirth 
*/

#include "LCD.h"
#include "Keypad.h"
#include "BME.h"
#include "GPS.h"
#include "SD2.h"
#include "ANE.h"

BME myBME = BME();
LCD myLCD = LCD();
GPS myGPS = GPS();
SD2 mySD = SD2();
ANE myAne = ANE();

//initialize keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {31, 33, 35, 37}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 18}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

uint32_t startTime = millis();

void setup() {
  Serial.begin(115200);
  Serial.println("BME Status: " + myBME.myStatus);
  Serial.println("GPS Status: " + myGPS.myStatus);
  myBME.start();
  myGPS.start();
  Serial.println("BME Status: " + myBME.myStatus);
  Serial.println("GPS Status: " + myGPS.myStatus);
  Serial.println("SD Status: " + mySD.myStatus);
}

void loop() {
  myLCD.showPage();
  char key = keypad.getKey();
  if (key != NO_KEY)
  {
    bool changed = myLCD.changePage(key);
  }
  if (myLCD.page == "home")
  {
    if (mySD.myStatus == "writing")
    {
      Serial.println("SD: Closing interupted file");
      mySD.closeFile();
    }
  }
  if (myLCD.page == "view2")
  {
    if (myLCD.view == "time")
    {
       myLCD.measurementString = myGPS.getTime();
       String currentTime = (String) myGPS.getTime();
       Serial.println("Time: "+ currentTime);
    }
    if (myLCD.view == "pressure")
    {
       myLCD.measurementValue = myBME.getPressure();
    }
    if (myLCD.view == "temperature")
    {
       myLCD.measurementValue = myBME.getTemperature();
    }
    if (myLCD.view == "gps")
    {
       myLCD.measurementString = myGPS.getLocation(false);
    }
    if (myLCD.view == "anemometer")
    {
      myLCD.measurementValue = myAne.getWindSpeed();
    }
    if (myLCD.view == "mic")
    {
      myLCD.measurementValue = analogRead(A7);
    }
  }
  if (myLCD.page == "rec2")
  {
    if (mySD.myStatus == "Card failed, or not present")
    {
      myLCD.changePage('#');
      Serial.println("ERROR: Card failed, or not present");
    }
    String currentTime = myGPS.getTime();
    String location = myGPS.getLocation(true);
    String fileName = "DAQ";
    String temperature = (String) myBME.getTemperature();
    String pressure = (String) myBME.getPressure();
    String windspeed = (String) myAne.getWindSpeed();
    if (millis() - startTime > 1000)
    {
      startTime = millis();
      mySD.writeData(fileName, currentTime, location, temperature, pressure, windspeed);
    }
  }
  if (myLCD.page == "recCenter")
  {
    String currentTime = myGPS.getTime();
    String location = myGPS.getLocation(true);
    String fileName = "DAQ";
    String temperature = (String) myBME.getTemperature();
    String pressure = (String) myBME.getPressure();
    String windspeed = (String) myAne.getWindSpeed();
    if (millis() - startTime > 1000)
    {
      startTime = millis();
      mySD.writeData(fileName, currentTime, location, temperature, pressure, windspeed);
    }
  }
}
