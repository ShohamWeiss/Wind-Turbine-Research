/*
  LCD.h - Library wrapping LiquidCrystal class.
  Created by Shoham Weiss, February 13, 2020.
*/

#ifndef LCD_h
#define LCD_h

#include "Arduino.h"
#include <LiquidCrystal.h>

class  LCD
{
  public:
    String page = "home";
    String view;
    float measurementValue;
    String measurementString;
    String dir;
    LCD();    
    bool changePage(char key);
    void showPage();
    
  private:
    //intitializing
    const int rs = 12, en = 11, data4 = 36, data5 = 34, data6 = 32, data7 = 30;
    int columnLCD, rowLCD;
    int columnTot = 16, rowTot = 2;
    LiquidCrystal lcd = LiquidCrystal(rs, en, data4, data5, data6, data7);
    
    //navigating
    void dispHome();
    void dispRecCenter();
    void dispRec();
    void dispRecFilename();
    void dispRec2();
    void dispView();
    void dispView2();
    
    //counting down
    unsigned long startMillis;
    int countDown = 9;
};

#endif
