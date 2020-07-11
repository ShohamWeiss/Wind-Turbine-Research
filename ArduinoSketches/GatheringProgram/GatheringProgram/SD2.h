/*
  SD2.h - Library wrapping SD class.
  Created by Shoham Weiss, February 23, 2020.
*/

#ifndef SD2_h
#define SD2_h

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

class SD2
{
  public:
    SD2();
    void writeData(String fileName, String currentTime, String location, String temperature, String pressure, String windspeed);
    void closeFile();
    void MakeFolder(String folderName);
    String myStatus;
   private:
    String folder;
    const int chipSelect = 53;
    File dataFile;
};
#endif
