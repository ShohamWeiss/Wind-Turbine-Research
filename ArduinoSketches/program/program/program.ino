#include <LiquidCrystal.h>
#include "Arduino.h"
#include "Keypad.h"

// initialize LCD
const int rs = 12, en = 11, data4 = 36, data5 = 34, data6 = 32, data7 = 30;
LiquidCrystal lcd(rs, en, data4, data5, data6, data7);
int columnLCD, rowLCD;

//initialize keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 6, 7, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
  // fire up the serial port.
  Serial.begin(9600);

  //initiallize the LCD
  initLCD();

}

String page = "Home";
void loop() {
  //get Key
  char key = keypad.getKey();
  //change page
  if (key != NO_KEY){
    lcd.clear();
    if (page == "Mic" || page == "BME") {
      if (key == '1') {
      page = page + "Record";
      } else if (key == '2') {
      page = page + "Display";
      }
    } else if (key == '1') {
      page = "Mic";
    } else if (key == '2') {
      page = "BME";
    } 
    if (key == '3') {
      page = "Home";
    }
  }
  //change display
  displayPage(page);
}

void initLCD() {
   // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // tell the user what's about to happen.
  Serial.println("Initialized LCD.");
}

void displayPage(String page) {
  if (page == "Home") {
      columnLCD = 0;
      rowLCD = 0;
      lcd.setCursor(columnLCD, rowLCD);
      lcd.print("1.Mic");
      rowLCD = 1;
      lcd.setCursor(columnLCD, rowLCD);
      lcd.print("2.BME");
  } else if (page == "Mic" || page == "BME") {
      columnLCD = 0;
      rowLCD = 0;
      lcd.setCursor(columnLCD, rowLCD);
      lcd.print("1.Record");
      rowLCD = 1;
      lcd.setCursor(columnLCD, rowLCD);
      lcd.print("2.Display 3.Back");
  } else if (page == "MicRecord") {
      double sample = getMicSample();
      lcd.setCursor(0, 0);
      lcd.print("Mic Values:");
      lcd.setCursor(0, 1);
      lcd.print(sample);
  } else {
      columnLCD = 0;
      rowLCD = 0;
      lcd.setCursor(columnLCD, rowLCD);
      lcd.print(page);
  }
}

double getMicSample() {
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
   const int sampleWindow = 500; // Sample window width in mS (50 mS = 20Hz)
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      unsigned int sample = analogRead(7);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = 50* (peakToPeak * 3.3) / 1024;  // convert to volts
   return volts;
}
