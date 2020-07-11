/*
  SD2.cpp - Library wrapping SD class.
  Created by Shoham Weiss, February 23, 2020.
*/

#include "Arduino.h"
#include "SD2.h"
#include <SPI.h>
#include <SD.h>

SD2::SD2()
{
  if (!SD.begin(chipSelect))
  {
    myStatus = "Card failed, or not present";
  } else
  {
    myStatus = "card initialized.";
  }
}

void SD2::MakeFolder(String folderName)
{
  folder = folderName;
  SD.mkdir(folderName);
  Serial.println("Writing Folder To SD: " + folderName);
}


void SD2::writeData(String fileName, String currentTime, String location, String temperature, String pressure, String windspeed)
{
  myStatus = "writing";
  int itir = 0;
  String itirFilename = fileName + itir;
  if (!dataFile)
  {
    while (SD.exists(itirFilename + ".csv"))
    {
        itir++;
        itirFilename = fileName + itir;
    }
    dataFile = SD.open(folder + "/" + itirFilename + ".csv", FILE_WRITE);
    if (dataFile) {
      String dataString = "Time,Latitude,Longitude,Temperature,Pressure,Windspeed";
      dataFile.println(dataString);
      Serial.println("Writing To SD: " + dataString);
    }
  }

  String dataString = currentTime + "," + location + "," + temperature + "," + pressure + "," + windspeed;
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    Serial.println("Writing To SD: " + dataString);
  }
}

void SD2::closeFile()
{ 
  if (dataFile) {  
    dataFile.close();
    myStatus = "ready";
    Serial.println("Closing file in SD");
  }
}
