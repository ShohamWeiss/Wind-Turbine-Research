/*
  LCD.cpp - Library wrapping LiquidCrystal class.
  Created by Shoham Weiss, February 13, 2020.
*/

#include "Arduino.h"
#include "LCD.h"
#include <LiquidCrystal.h>

LCD::LCD()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(columnTot, rowTot);
}

bool LCD::changePage(char key)
{
  bool canChange = false;
  if (page == "home") ////////
  {
    if (key == '1')
    {
      startMillis = millis();
      countDown = 9;
      page = "recCenter";
      canChange = true;
    }
    if (key == '2')
    {
      page = "rec";
      canChange = true;
    }
    if (key == '3')
    {
      page = "view";
      canChange = true;
    }
  }
  else if (page == "recCenter") ////////
  {
    if (key == '#')
    {
      page = "home";
      canChange = true;
    }
  }
  else if (page == "rec") ////////
  {
    if (key == '1')
    {
      countDown = 30 * 1;
      page = "rec2";
      canChange = true;
    }
    if (key == '2')
    {
      countDown = 60 * 1;
      page = "rec2";
      canChange = true;
    }
    if (key == '3')
    {
      countDown = 60 * 4;
      page = "rec2";
      canChange = true;
    }
    if (key == '#')
    {
      page = "home";
      canChange = true;
    }
  }
  else if (page == "setfilename")
  {
      if (key == '1')
      {
          dir = "NW";
          canChange = true;
      }
      if (key == '2')
      {
          dir = "N";
          canChange = true;
      }
      if (key == '3')
      {
          dir = "NE";
          canChange = true;
      }
      if (key == '4')
      {
          dir = "W";
          canChange = true;
      }
      if (key == '6')
      {
          dir = "E";
          canChange = true;
      }
      if (key == '7')
      {
          dir = "SW";
          canChange = true;
      }
      if (key == '8')
      {
          dir = "S";
          canChange = true;
      }
      if (key == '9')
      {
          dir = "SE";
          canChange = true;
      }
      if (key == '#')
      {
          page = "home";
          canChange = true;
      }
      if (key != '*' && key != '0' && key != '5')
      {
          page = "rec2";
          canChange = true;
      }
  }
  else if (page == "rec2") ////////
    {
      if (key == '#')
      {
        page = "home";
        canChange = true;
      }
    }
  else if (page == "view")
  {
    if (key == '1')
    {
      page = "view2";
      view = "time";
      canChange = true;
    }
    if (key == '2')
    {
      page = "view2";
      view = "temperature";
      canChange = true;
    }
    if (key == '3')
    {
      page = "view2";
      view = "pressure";
      canChange = true;
    }
    if (key == '4')
    {
      page = "view2";
      view = "mic";
      canChange = true;
    }
    if (key == '5')
    {
      page = "view2";
      view = "gps";
      canChange = true;
    }
    if (key == '6')
    {
      page = "view2";
      view = "anemometer";
      canChange = true;
    }
    if (key == '#')
    {
      page = "home";
      canChange = true;
    }
  }
  else if (page == "view2") ////////
  {
    if (key == '#')
    {
      page = "home";
      canChange = true;
      measurementValue = 0;
      measurementString = "";
    }
  }
  if (canChange) ////////////////
  {
    lcd.clear();
    Serial.println("+++++ Changed Page: " + page + " +++++");
  } else
  {
    Serial.println("----- Stayed in page: " + page + " with key: " + key + " -----"); 
  }
  return canChange;
}

void LCD::showPage()
{
  if (page == "home")
  {
    dispHome();
  }
  if (page == "recCenter")
  {
    dispRecCenter();
  }
  if (page == "rec")
  {
    dispRec();
  }
  if (page == "setfilename")
  {
    dispRecFilename();
  }
  if (page == "rec2")
  {
    dispRec2();
  }
  if (page == "view")
  {
    dispView();
  }
  if (page == "view2")
  {
    dispView2();
  }
}

void LCD::dispHome()
{
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("1.Rec Center");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("2.Rec 3.view");
}

void LCD::dispRecCenter()
{
  if (countDown == 0)
  {
    page = "home";
    return;
  }
  if (millis() - startMillis >= 1000)
  {
    countDown--;
    startMillis = millis();
  }
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("Count Down");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print(countDown);
}

void LCD::dispRec()
{
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("How long?1. 30sc ");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("2. 1min  3. 4min");
}

void LCD::dispRecFilename()
{
    columnLCD = 0;
    rowLCD = 0;
    lcd.setCursor(columnLCD, rowLCD);
    lcd.print("Direction?");
    rowLCD = 1;
    lcd.setCursor(columnLCD, rowLCD);
    lcd.print("1.NW 2.N 3.NE");
}

void LCD::dispRec2()
{
  if (countDown == 0)
  {
    page = "home";
    return;
  }
  if (millis() - startMillis >= 1000)
  {
    countDown--;
    startMillis = millis();
  }
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("Recording");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print(countDown);
}

void LCD::dispView()
{
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("1.Ti 2.Te 3.Pr");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print("4.Mi 5.Gp 6.An");
}

void LCD::dispView2()
{
  lcd.clear();
  columnLCD = 0;
  rowLCD = 0;
  lcd.setCursor(columnLCD, rowLCD);
  lcd.print(view + ":");
  rowLCD = 1;
  lcd.setCursor(columnLCD, rowLCD);
  if (measurementValue == 0)
  {
    lcd.print(measurementString);
  } else if (measurementString == "")
  {
    lcd.print(measurementValue);
  }
}
