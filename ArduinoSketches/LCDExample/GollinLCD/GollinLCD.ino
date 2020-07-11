/*************************************************** 
  This file is LCD.ino
  
  It will send a few lines to the serial monitor window and display
  information on a 16 x 2 liquid crystal display.

  George Gollin
  University of Illinos at Urbana-Champaign
  September 2018

  Make sure you have the correct port selected: go to 
    Tools -> Port...

  Also go to Tools -> Serial Monitor and set the baud rate to 9600.

  See https://www.arduino.cc/en/Serial/Print and
  https://www.arduino.cc/en/Reference/LiquidCrystalPrint
  for more information.

 ****************************************************/

/////////////////////////// LCD ///////////////////////////////

// LCD header file.
#include <LiquidCrystal.h>

// initialize the LCD library by associating LCD interface pins
// with the arduino pin numbers to which they are connected
const int rs = 12, en = 11, data4 = 36, data5 = 34, data6 = 32, data7 = 30;

// now instantiate (create an instance of) a LiquidCrystal object.
LiquidCrystal lcd(rs, en, data4, data5, data6, data7);

// a global variable, accessible to all functions:
int lines_written;

//////////////// setup function is automatically executed once ///////////////////

void setup() {

  // fire up the serial port so we can write/read to/from the serial monitor window.
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // tell the user what's about to happen.
  Serial.println("About to write to LCD.");

  // a little more information for the user...
  Serial.println("LCD lines are 16 characters long.");

  // initialize the number of lines written...
  lines_written = 0;

}

//////////////// loop function is executed repeatedly ///////////////////

void loop() {

  // integers specifing column and row in the LCD
  int column, row;
  
  // increment the counter
  lines_written += 1;
  
  // Print a message to the LCD's zeroth line, beginning at the zeroth character.
  column = 0;
  row = 0;

  lcd.setCursor(column, row);
  lcd.print("writes: ");

  // now print how many times we've written a pair of lines.
  lcd.print(lines_written);

  // now do the same thing, but for a floating point number on the next LCD line.

  float lcd_float_test = lines_written; 
  row = 1;
  lcd.setCursor(column, row);

  lcd.print("float: ");
  lcd.print(lcd_float_test);
  
  // now pause for 250 milliseconds  
  delay(250);
  
}
