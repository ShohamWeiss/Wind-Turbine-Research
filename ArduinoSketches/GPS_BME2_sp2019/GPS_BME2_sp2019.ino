/*
   
  file: GPS_BME2_sp2019.ino

  Read data from a BME680 pressure/temperature/humidity sensor, as
  well as an Adafruit Ultimate GPS breakot board.

  You should set the serial monitor baud rate to 115,200 and wire the 
  circuit as shown jin the physics 398DLP data logger schematic.

  George Gollin
  University of Illinois
  February 2019

*/

//////////////////////////////////////////////////////////////////////
/////////////////////// global parameters ////////////////////////////
//////////////////////////////////////////////////////////////////////

/////////////////////////// LCD parameters ///////////////////////////

#include <LiquidCrystal.h>

// The LCD is a GlobalFontz 16 x 2 device.

// initialize the LCD library by associating LCD interface pins
// with the arduino pins to which they are connected. first define
// the pin numbers: reset (rs), enable (en), etc.
const int rs = 12, en = 11, d4 = 36, d5 = 34, d6 = 32, d7 = 30;

// instantiate an LCD object named "lcd" now. We can use its class
// functions to do good stuff, e.g. lcd.print("the text").
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// total number of characters in one line of the LCD display
#define LCDWIDTH 16

//////////////////////////// keypad parameters ////////////////////////////

#include <Keypad.h>

// keypad has four rows and three columns. pushing one key connects together
// the corresponding row and column pins.

const byte ROWS = 4;
const byte COLS = 3;

// what the keys actually stand for:
char keys[ROWS][COLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};

// I am using an Arduino Mega 2560 with a number of breakout boards. 
// I have the following  pin assignments in order to allow the column
// pins to generate interrupts, if I should decide to take awareness of
// the keypad this way. Note that there might be complicated interactions
// between interrupts used to read the microphone's ADC channel and keypad 
// interrupts. 
 
// Arduino pins looking at the three column pins:
byte colPins[COLS] = {2, 3, 18}; 
// Arduino pins looking at the four row pins:
byte rowPins[ROWS] = {31, 33, 35, 37}; 

// instantiate a keypad object.
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// character returned when I query the keypad (might be empty)
char query_keypad_char = NO_KEY;

// last non-null character read from keypad
char last_key_char = NO_KEY;

/////////////////////////////// some libraries ///////////////////////////////////

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
///////////////#include "SdFat.h"
#include "AnalogBinLogger.h"
#include <bme680.h>
#include <bme680_defs.h>
// include the SD library:
#include <SD.h>

////////////////////////// DS3231 real time clock parameters ////////////////////////

// this is an I2C device on an Adafruit breakout board. It is a cute little thing 
// with a backup battery.

#include "RTClib.h"

// instantiate a real time clock object named "rtc":
RTC_DS3231 rtc;

// names of the days of the week:
char daysOfTheWeek[7][4] = 
  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

///////////////////////// BME680 T/P/RH/VOC sensor ////////////////////////

// some pins, not needed for I2C.
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// sea level pressure definition
#define SEALEVELPRESSURE_HPA (1013.25)

// instantiate a BME680 object as an I2C device.
Adafruit_BME680 bme; // I2C

//do the following if hardware SPI will be used instead of I2C.
//Adafruit_BME680 bme(BME_CS);
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

// Set BME680ECHO to 'false' to turn off echoing the BME680 data.
#define BME680ECHO false



////////////////////////////////// microSD card ////////////////////////////////
// chip select pin
const int microSD_CS_pin = 53;

// clock 
const int microSD_CLK_pin = 52;

// data out (DO or SO or MISO or...)
const int microSD_DO_pin = 50;

// data in (DI or SI or MOSI or...)
const int microSD_DI_pin = 51;

// instantiate a file object.
File myFile;

/////////////////////////////// GPS parameters /////////////////////////////

// You'll want to do Tools -> Serial Monitor, then set the baud rate to 115,200
// if you want to stream GPS data to the serial monitor window. If you only set
// the baud rate to 9600 it'll be too slow, and interfere with data transmission
// from the GPS device to the Arduino processor.

// Also make sure that Tools -> Port is set to the Arduino's port.

// We are interested in two kinds of "sentences" holding GPS navigational data: one
// begins with $GPRMC, which holds "Recommended minimum specific GPS/Transit data."
// The other begins with $GPGGA, which holds "Global Positioning System Fix Data."
// Both include latitude and longitude. See a description of the formats, below. 

// here is the $GPRMC format:
// 0 - 5     $GPRMC
// 7 - 8     hour, UTC      
// 9 - 10    minutes, UTC      
// 11 - 12   seconds, UTC
// 14 - 16   milliseconds
// 18        A for navigation data OK, V for navigation data not present.
// 20 - 28   latitude x 100, in degrees; F9.4 format      
// 30        N or S
// 32 - 41   longitude x 100, in degrees; F10.4 format      
// 43        E or W
// next      speed over the ground, in knots; floating point, 5 characters
//           (data between 7th and 8th commas) 
// next      course direction, degrees, floating point
//           (data between 8th and 9th commas) 
// other stuff too. Two examples, the first with navigation information, the second without:
// $GPRMC,135228.000,A,4006.9605,N,08815.4528,W,0.44,336.27,090618,,,A*71
// $GPRMC,135217.000,V,,,,,0.74,222.59,090618,,,N*45
//           1         2         3         4         5         6         7         8
// 012345678901234567890123456789012345678901234567890123456789012345678901234567890

// here is the $GPGGA format:
// 0 - 5     $GPGGA
// 7 - 8     hour, UTC      
// 9 - 10    minutes, UTC      
// 11 - 16   seconds, UTC, F6.3 format      
// 18 - 26   latitude x 100, in degrees; F9.4 format      
// 28        N or S
// 30 - 39   longitude x 100, in degrees; F10.4 format      
// 41        E or W
// 43        fix quality: 0 for no fix, 1 for GPS, 2 for DGPS.
// 45 - 46   number of satellites
// 48 - 51   horizontal dilution of precision
// 53 - 57   altitude above sea level
// 59        M for altitude in meters
// other stuff too. Two examples, the first with navigation information, the second without:
// $GPGGA,135224.000,4006.9611,N,08815.4523,W,1,07,1.11,211.7,M,-33.9,M,,*53
// $GPGGA,135217.000,,,,,0,07,,,M,,M,,*7C
//           1         2         3         4         5         6         7         8
// 012345678901234567890123456789012345678901234567890123456789012345678901234567890

// Set GPSECHO_GPS_query to 'false' to turn off echoing the GPS data to the Serial console  
// from the GPS_query function. (Set it to true when debugging.)
#define GPSECHO_GPS_query false

// a similar debugging flag for loop():
#define GPSECHO_loop true

// get the header file in which lots of things are defined:
#include <Adafruit_GPS.h>

// also define some more stuff relating to update rates. See 
// https://blogs.fsfe.org/t.kandler/2013/11/17/set-gps-update-
// rate-on-arduino-uno-adafruit-ultimate-gps-logger-shield/
#define PMTK_SET_NMEA_UPDATE_10SEC "$PMTK220,10000*2F"
#define PMTK_SET_NMEA_UPDATE_5SEC "$PMTK220,5000*2F"

// let's use the Arduino's second serial port to communicate with the GPS device.
#define GPSSerial Serial2

// Connect to the GPS via the Arduino's hardware port
Adafruit_GPS GPS(&GPSSerial);
     
// we don't expect a valid GPS "sentence" to be longer than this...
#define GPSMAXLENGTH 120
// or shorter than this:
#define GPSMINLENGTH 55

// last sentence read from the GPS:
char* GPS_sentence;

// we'll also want to convert the character array to a string for convenience
String GPS_sentence_string;

// a string to hold what kind of GPS sentence we've received
String GPS_command;

// pointers into parts of a GPRMC GPS data sentence:

const int GPRMC_hour_index1 = 8;
const int GPRMC_hour_index2 = GPRMC_hour_index1 + 2;

const int GPRMC_minutes_index1 = GPRMC_hour_index2;
const int GPRMC_minutes_index2 = GPRMC_minutes_index1 + 2;
      
const int GPRMC_seconds_index1 = GPRMC_minutes_index2;
const int GPRMC_seconds_index2 = GPRMC_seconds_index1 + 2;
      
const int GPRMC_milliseconds_index1 = GPRMC_seconds_index2 + 1;   // skip the decimal point
const int GPRMC_milliseconds_index2 = GPRMC_milliseconds_index1 + 3;
      
const int GPRMC_AV_code_index1 = 19;
const int GPRMC_AV_code_index2 = GPRMC_AV_code_index1 + 1;
      
const int GPRMC_latitude_1_index1 = 21;
const int GPRMC_latitude_1_index2 = GPRMC_latitude_1_index1 + 4;
      
const int GPRMC_latitude_2_index1 = GPRMC_latitude_1_index2 + 1;   // skip the decimal point
const int GPRMC_latitude_2_index2 = GPRMC_latitude_2_index1 + 4;

const int GPRMC_latitude_NS_index1 = 31;
const int GPRMC_latitude_NS_index2 = GPRMC_latitude_NS_index1 + 1;

const int GPRMC_longitude_1_index1 = 33;
const int GPRMC_longitude_1_index2 = GPRMC_longitude_1_index1 + 5;    // 0 - 180 so we need an extra digit
      
const int GPRMC_longitude_2_index1 = GPRMC_longitude_1_index2 + 1;   // skip the decimal point
const int GPRMC_longitude_2_index2 = GPRMC_longitude_2_index1 + 4;
      
const int GPRMC_longitude_EW_index1 = 44;
const int GPRMC_longitude_EW_index2 = GPRMC_longitude_EW_index1 + 1;

// pointers into a GPGGA GPS data sentence:

const int GPGGA_hour_index1 = 8;
const int GPGGA_hour_index2 = GPGGA_hour_index1 + 2;

const int GPGGA_minutes_index1 = GPGGA_hour_index2;
const int GPGGA_minutes_index2 = GPGGA_minutes_index1 + 2;
      
const int GPGGA_seconds_index1 = GPGGA_minutes_index2;
const int GPGGA_seconds_index2 = GPGGA_seconds_index1 + 2;
      
const int GPGGA_milliseconds_index1 = GPGGA_seconds_index2 + 1;   // skip the decimal point
const int GPGGA_milliseconds_index2 = GPGGA_milliseconds_index1 + 3;
      
const int GPGGA_latitude_1_index1 = 19;
const int GPGGA_latitude_1_index2 = GPGGA_latitude_1_index1 + 4;
      
const int GPGGA_latitude_2_index1 = GPGGA_latitude_1_index2 + 1;   // skip the decimal point
const int GPGGA_latitude_2_index2 = GPGGA_latitude_2_index1 + 4;

const int GPGGA_latitude_NS_index1 = 29;
const int GPGGA_latitude_NS_index2 = GPGGA_latitude_NS_index1 + 1;

const int GPGGA_longitude_1_index1 = 31;
const int GPGGA_longitude_1_index2 = GPGGA_longitude_1_index1 + 5;    // 0 - 180 so we need an extra digit
      
const int GPGGA_longitude_2_index1 = GPGGA_longitude_1_index2 + 1;   // skip the decimal point
const int GPGGA_longitude_2_index2 = GPGGA_longitude_2_index1 + 4;
      
const int GPGGA_longitude_EW_index1 = 42;
const int GPGGA_longitude_EW_index2 = GPGGA_longitude_EW_index1 + 1;

const int GPGGA_fix_quality_index1 = 44;
const int GPGGA_fix_quality_index2 = GPGGA_fix_quality_index1 + 1;

const int GPGGA_satellites_index1 = 46;
const int GPGGA_satellites_index2 = GPGGA_satellites_index1 + 2;

// keep track of how many times we've read a character from the GPS device. 
long GPS_char_reads = 0;

// bail out if we exceed the following number of attempts. when set to 1,000,000 this corresponds
// to about 6 seconds. we need to do this to keep an unresponsive GPS device from hanging the program.
const long GPS_char_reads_maximum = 1000000;

// define some of the (self-explanatory) GPS data variables. Times/dates are UTC.
String GPS_hour_string;
String GPS_minutes_string;
String GPS_seconds_string;
String GPS_milliseconds_string;
int GPS_hour;
int GPS_minutes;
int GPS_seconds;
int GPS_milliseconds;

// this one tells us about data validity: A is good, V is invalid.
String GPS_AV_code_string;

// latitude data
String GPS_latitude_1_string;
String GPS_latitude_2_string;
String GPS_latitude_NS_string;
int GPS_latitude_1;
int GPS_latitude_2;

// longitude data
String GPS_longitude_1_string;
String GPS_longitude_2_string;
String GPS_longitude_EW_string;
int GPS_longitude_1;
int GPS_longitude_2;

// velocity information; speed is in knots! 
String GPS_speed_knots_string;
String GPS_direction_string;
float GPS_speed_knots;
float GPS_direction;

String GPS_date_string;

String GPS_fix_quality_string;
String GPS_satellites_string;
int GPS_fix_quality;
int GPS_satellites;

String GPS_altitude_string;
float GPS_altitude;

// Let's limit the numnber of times we will spin thrugh the "loop" function before
// closing the data file and parking the program in an infinite loop.
const long maximum_times_to_loop = 200;
long my_counter;


//////////////////////////////////////////////////////////////////////
//////////////////// end of global parameters ////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
///////////////////////// setup function /////////////////////////////
//////////////////////////////////////////////////////////////////////

void setup() 
{

  // setup is run once, immediately after the program is downloaded into
  // the Arduino. 

  /////////////////// serial port for serial monitor window ///////////////////

  // set to 115,200 baud so that we don't cause problems associated with reading
  // and reporting GPS data.
  Serial.begin(115200);
  while (!Serial)
  {
    
  }

  my_counter = 0;
  
 /////////////////////////// LCD setup ////////////////////////////

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD.
  //           0123456789012345    0123456789012345
  LCD_message("Physics 398DLP  ", "GPS/BME/microSD ");

  // delay a bit so I have time to see the display.
  delay(1000);
  
/////////////////////////// DS3231 real time clock setup /////////////////////////

  // turn on the RTC and check that it is talking to us.
/*
  if (! rtc.begin()) {

    // uh oh, it's not talking to us.
    LCD_message("DS3231 RTC", "unresponsive");

    // delay 5 seconds so that user can read the display
    delay(5000);  

    } else {

  if (rtc.lostPower()) {
  
    LCD_message("DS3231 RTC lost", "power. Set to...");

    // the following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Set the RTC with an explicit date & time: September 1, 1988, 7:37:00 am
    // rtc.adjust(DateTime(1988, 9, 1, 7, 37, 0));
    
    }
  }
*/
/////////////////////////// BME680 setup /////////////////////////

  // turn on the BME680 and check that it is talking to us.

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization

  /*   
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  */

  // set oversamplings to 1 to speed everything up. We can disable a
  // measurement by setting the oversampling to BME680_OS_NONE.
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setPressureOversampling(BME680_OS_1X);
  
  // set the "Infinite Impulse Response Filter" size to zero to disable
  // filtering. 
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);

  // set both the heater temperature (first argument) and heating duration 
  // (second argument) to zero to disable the use of the VOC gas sensor.
  bme.setGasHeater(0, 0); 

  // when only the temperature measurement is enabled, one measurement 
  // takes about 18.7 milliseconds. When T, P, and H are enabled, the 
  // trio of measurements takes about 27.4 miliseconds. If I set 
  // the oversampling to 2 instead of 1 the time per trio of measurements
  // increases to approximately 41 milliseconds.

/////////////////////////// microSD setup /////////////////////////

  // turn on the microSD card.

  pinMode(microSD_CS_pin, OUTPUT);

  if (!SD.begin(microSD_CS_pin)) {
    Serial.println("Uh oh... microSD initialization failed!");
    while (1);
    }

////////////////////////////////// GPS setup //////////////////////////////

  // If you want to see a detailed report of the GPS information, open a serial 
  // monitor window by going to the Tools -> Serial Monitor menu, then checking
  // the Autoscroll box at the bottom left of the window, and setting the baud rate
  // to 115,200 baud.

  // once we've set up the GPS it will free-run: it has its own built-in microprocessor.
     
  // 9600 NMEA is the default communication and baud rate for Adafruit MTK GPS units. 
  // NMEA is "National Marine Electronics Association." 
  // Note that this serial communication path is different from the one driving the serial 
  // monitor window on your laptop, which should be running at 115,200 baud.
  GPS.begin(9600);
  
  // turn on RMC (recommended minimum) and GGA (fix data, including altitude)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  // uncomment this line to turn on only the "minimum recommended" data:
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  
  // Set the update rate to once per second. Faster than this might make it hard for
  // the serial communication line to keep up: you'll need to check this.
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 
  
  // You'll want to check that the faster read rates work reliably for you before
  // using them. The readout rates are, of course, 1, 2, 5, and 10 Hz.
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_2HZ); 
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); 
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); 
  
  // Uncomment this to set the update rate to once per 5 seconds.
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5SEC);
     
  // Uncomment this to set the update rate to once per 10 seconds.
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10SEC);
     
  // Request updates on antenna status, comment out to keep quiet.
  // GPS.sendCommand(PGCMD_ANTENNA);

  // Ask for firmware version, write this to the serial line. Comment out to keep quiet.
  // GPSSerial.println(PMTK_Q_RELEASE);

  ////////////////////////////// say hello ///////////////////////////

  // write a message to the LCD
  LCD_message("Hi there.", "GPS explorations");

  /////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////
//////////////////////////// loop function ///////////////////////////
//////////////////////////////////////////////////////////////////////

void loop() 
{
  // loop function is executed repeatedly after the setup function finishes.
  my_counter++;

  //////////////////// file opening  //////////////////// 
  
  // have I entered loop for the first time? If so, open a microSD file.
  if(my_counter == 1)
  {
    // open the file for writing. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    myFile = SD.open("GPSBME2.txt", FILE_WRITE);

    // delay a bit so there's time for the file to open
    delay(1000);   
  }

  //////////////////// file closing  //////////////////// 
  
  // have I entered loop for the last time? If so, close the microSD file.
  if(my_counter > maximum_times_to_loop)
  {

    // close the file:
    myFile.close();
    Serial.println("Just closed the microSD file.");

  //////////////////// now park the Arduino  //////////////////// 

    // tumble into an infinite loop.
    Serial.println("All done, so about to enter a parking loop.");
    while(true){}
  
  }
  
  //////////////////// read the BME680 here /////////////////////// 

  // read, and complain if there's a problem.
  if (!bme.performReading()) {
    Serial.println("Failed to perform BME680 reading :(");
    return;
    }
  
  float BME680_T = bme.temperature;
  float BME680_P = bme.pressure;
  float BME680_H = bme.humidity;

  // echo what we got to the serial monitor, perhaps.
  if(BME680ECHO)
  {
    Serial.print("BME680 temperature = "); Serial.print(BME680_T); Serial.println("*C");
    Serial.print("BME680 pressure = "); Serial.print(BME680_P); Serial.println("Pa (N/m^2)");
    Serial.print("BME680 humidity = "); Serial.print(BME680_H); Serial.println(" %");
  }

  //////////////////// read another sensor here ///////////////// 

  // if you have other devices, read from them here.
  
  //////////////////// write stuff to the microSD card ////////// 

  // do the writes as CSV.
  myFile.print("T_P_H,"); 
  myFile.print(BME680_T); myFile.print(",");
  myFile.print(BME680_P); myFile.print(",");
  myFile.print(BME680_H); myFile.print(",");  
  myFile.print("my_counter,"); myFile.println(my_counter); 
  
  //////////////////// now get GPS information. ///////////////// 

  // If we get a complete navigational sentence (so that GPS_query 
  // returns the value zero) write the navigation information to the 
  // microSD memory. 
  
  int GPS_return_code = GPS_query();

  if(GPS_return_code == 0)
    {

    // put your microSD GPS data writing code here. 

    // The (global) variables filled by GPS_query are these:
    //      GPS_hour, GPS_minutes, GPS_seconds, GPS_milliseconds, 
    //      GPS_latitude_1, GPS_latitude_2, GPS_latitude_NS_string,
    //      GPS_longitude_1, GPS_longitude_2, GPS_longitude_EW_string, 
    //      GPS_speed_knots, GPS_direction, GPS_satellites, GPS_altitude
    
    myFile.print("GPS_hour,"); myFile.print(GPS_hour); myFile.print(","); 
    myFile.print("GPS_minutes,"); myFile.print(GPS_minutes); myFile.print(",");
    myFile.print("GPS_seconds,"); myFile.print(GPS_seconds); myFile.print(","); 
    myFile.print("GPS_milliseconds,"); myFile.println(GPS_milliseconds);  
    
    myFile.print("GPS_latitude_1,"); myFile.print(GPS_latitude_1);  myFile.print(",");
    myFile.print("GPS_latitude_2,"); myFile.print(GPS_latitude_2);  myFile.print(",");
    myFile.print("GPS_latitude_NS_string,"); myFile.print(GPS_latitude_NS_string);  myFile.print(",");
    myFile.print("GPS_longitude_1,"); myFile.print(GPS_longitude_1);  myFile.print(",");
    myFile.print("GPS_longitude_2,"); myFile.print(GPS_longitude_2);  myFile.print(",");
    myFile.print("GPS_longitude_EW_string,"); myFile.println(GPS_longitude_EW_string); 
    
    // Now print a summary of the GPS data and parsed results, if we want.

    if(GPSECHO_loop)
      {
      Serial.print("\nGPS sentence: "); Serial.println(GPS_sentence_string);
  
      Serial.print("Time (UTC) = "); 

      if(GPS_hour < 10) Serial.print("0");
      Serial.print(GPS_hour); Serial.print(":");      

      if(GPS_minutes < 10) Serial.print("0");
      Serial.print(GPS_minutes); Serial.print(":");
  
      if(GPS_seconds < 10) Serial.print("0");
      Serial.print(GPS_seconds); Serial.print(".");
      
      Serial.println(GPS_milliseconds);
    
      Serial.print("Latitude x 100 = "); Serial.print(GPS_latitude_1); Serial.print(".");
      Serial.print(GPS_latitude_2); Serial.print(" "); Serial.print(GPS_latitude_NS_string);
  
      Serial.print("    Longitude x 100 = "); Serial.print(GPS_longitude_1); Serial.print(".");
      Serial.print(GPS_longitude_2); Serial.print(" "); Serial.println(GPS_longitude_EW_string); 
  
      Serial.print("Speed (knots) = "); Serial.print(GPS_speed_knots);
      Serial.print("     Direction (degrees) = "); Serial.println(GPS_direction);
  
      // $GPGGA sentences include the number of satellites and altitude. $GPRMC does not.
      if(GPS_command.equals("$GPGGA"))
        {
        Serial.print("Number of satellites: "); Serial.print(GPS_satellites);
        Serial.print("       Altitude (meters): "); Serial.println(GPS_altitude);
        }
  
      if(GPS_command.equals("$GPRMC"))
        {
        Serial.println("$GPRMC sentences do not include the number of satellites or altitude.");
        }       
      }
    }

 
}   // end of loop().

//////////////////////////////////////////////////////////////////////
////////////////////// GPS_query function /////////////////////////
//////////////////////////////////////////////////////////////////////

int GPS_query()
{

  // return 0 if we found good GPS navigational data and -1 if not. we might
  // return a -1 when there is an error in the data, or when the sentence
  // doesn't carry navigational information.
  
  // The GPS device has its own microprocessor and, once we have loaded its parameters,
  // free-runs at a fixed sampling rate. We do not trigger its registration of
  // latitude and longitude, rather we just read from it the last data record
  // it has stored. And we do it one character at a time!

  // I will ask the GPS for a single character, which will be an ASCII null ('\0')
  // if there are no data to be read, and something else if there are data present.
  // If I get an ASCII null, just return immediately with a -1 return code. If I DO find
  // some data, then keep reading it until we are finished, or else hit another ASCII null.

  // When the GPS has yielded a complete, entire sentence of navigation data, we'll parse 
  // it and load that information into some global variables. 

  // initialize the number-of-reads counter. 
  GPS_char_reads = 0;

  // This gets the last sentence read from GPS and clears a newline flag in the Adafruit 
  // library code. If there aren't any data, we'll get an ASCII null.
  GPS_sentence = GPS.lastNMEA();  

  // Stay inside the following loop until we've read a complete GPS sentence with
  // good navigational data, or else the loop times out. With GPS_char_reads_maximum 
  // set to a million this'll take about 6 seconds to time out. 

  while (true) {
  
    while(GPS_char_reads <= GPS_char_reads_maximum) 
      {
  
      // try to read a single character from the GPS device. we'll get an ascii 
      // null ('\0') if there's nothing to read, either becauser we're reading 
      // to fast for the GPS to keep up, or becauser there's not data to repprt.
      char single_GPS_char = GPS.read();
  
      if(single_GPS_char == '\0')
        {
        // Serial.println("\ncharacter was a null so bail out.");
        return -1;
        }
  
      // echo the character to the screen.
      if(GPSECHO_GPS_query) Serial.print(single_GPS_char);
      
      // bump the number of times we've tried to read from the GPS.
      GPS_char_reads++;
  
      // now ask if we've received a complete data sentence. If yes, break
      // out of the "while(GPS_char_reads <= GPS_char_reads_maximum)" loop. 
      
      if(GPS.newNMEAreceived()) break;
  
      }
    
    // if we've landed here because we hit the limit on the number of character reads we've 
    // tried, print a message and bail out.
    
    if (GPS_char_reads >= GPS_char_reads_maximum) 
      {
      Serial.println("Having trouble reading GPS navigation data. Try again later.");
      LCD_message("GPS navigation  ", "data unavailable");    
      return -1;        
      }

    // Land here because we have received a complete GPS sentence and executed the "break" 
    // in the above while loop. get the last complete sentence read from the GPS.
    GPS_sentence = GPS.lastNMEA();
    
    // convert GPS data sentence from a character array to a string.
    GPS_sentence_string = String(GPS_sentence);
  
    if (GPSECHO_GPS_query) 
      {
      Serial.println("\n******************\njust received a complete sentence, so parse stuff. Sentence is");
      Serial.println(GPS_sentence_string);
      }
  
    // now do a cursory check that the sentence we've just read is OK. Check that there is only
    // one $, as the first character in the sentence, and that there's an asterisk (which comes 
    // immediately before the checksum).
     
    // sentence starts with a $? 
    bool data_OK = GPS_sentence_string.charAt(1) == '$';    
  
    // sentence contains no other $? The indexOf call will return -1 if $ is not found.
    data_OK = data_OK && (GPS_sentence_string.indexOf('$', 2) <  0);
    
    // now find that asterisk...
    data_OK = data_OK && (GPS_sentence_string.indexOf('*', 0) >  0);
  
    // now parse the GPS sentence. I am only interested in sentences that begin with
    // $GPGGA ("GPS fix data") or $GPRMC ("recommended minimum specific GPS/Transit data").
  
    if(GPSECHO_GPS_query)
      {
      Serial.print("length of GPS_sentence_string just received...");
      Serial.println(GPS_sentence_string.length());
      }
  
    // now get substring holding the GPS command. Only proceed if it is $GPRMC or $GPGGA.
    GPS_command = GPS_sentence_string.substring(0, 7);
  
    // also trim it to make sure we don't have hidden stuff or white space sneaking in.
    GPS_command.trim();
  
    if(GPSECHO_GPS_query) 
      {
      Serial.print("GPS command is "); Serial.println(GPS_command);
      }   
  
    // if data_OK is true then we have a good sentence. but we also need the sentence
    // to hold navigational data we can use. we can only work with GPRMC and GPGGA sentences. 
  
    bool command_OK = GPS_command.equals("$GPRMC") || GPS_command.equals("$GPGGA"); 
    
    // if we have a sentence that, upon cursory inspection, is well formatted AND might
    // hold navigational data, continue to parse the sentence. If the GPS device
    // hasn't found any satellites yet, we'll want to bail out.
  
     if (!command_OK) 
       {

       if(GPSECHO_GPS_query) 
         {Serial.println("GPS sentence isn't a navigational information sentence.");
         LCD_message("GPS navigation  ", "data unavailable");    
        
       }

      return -1;        
      }
      
    //////////////////////////////////////////////////////////////////////
    /////////////////////////// GPRMC sentence ///////////////////////////
    //////////////////////////////////////////////////////////////////////
    
     if (data_OK && GPS_command.equals("$GPRMC"))
        {
            
        if(GPSECHO_GPS_query) 
          {
          Serial.print("\nnew GPS sentence: "); Serial.println(GPS_sentence_string);
          }
    
        // parse the time
    
        GPS_hour_string = GPS_sentence_string.substring(GPRMC_hour_index1, GPRMC_hour_index2);
        GPS_minutes_string = GPS_sentence_string.substring(GPRMC_minutes_index1, GPRMC_minutes_index2);
        GPS_seconds_string = GPS_sentence_string.substring(GPRMC_seconds_index1, GPRMC_seconds_index2);
        GPS_milliseconds_string = GPS_sentence_string.substring(GPRMC_milliseconds_index1, 
        GPRMC_milliseconds_index2);
        GPS_AV_code_string = GPS_sentence_string.substring(GPRMC_AV_code_index1, GPRMC_AV_code_index2);
    
        GPS_hour = GPS_hour_string.toInt();
        GPS_minutes = GPS_minutes_string.toInt();
        GPS_seconds = GPS_seconds_string.toInt();
        GPS_milliseconds = GPS_milliseconds_string.toInt();
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Time (UTC) = "); Serial.print(GPS_hour); Serial.print(":");
          Serial.print(GPS_minutes); Serial.print(":");
          Serial.print(GPS_seconds); Serial.print(".");
          Serial.println(GPS_milliseconds);
          Serial.print("A/V code is "); Serial.println(GPS_AV_code_string);
          }
    
        // now see if the data are valid: we'll expect an "A" as the AV code string.
        // We also expect an asterisk two characters from the end. Also check that the sentence 
        // is at least as long as the minimum length expected.
    
        data_OK = GPS_AV_code_string == "A";
    
        // now look for the asterisk after trimming any trailing whitespace in the GPS sentence.
        // the asterisk preceeds the sentence's checksum information, which I won't bother to check.
        int asterisk_should_be_here = GPS_sentence_string.length() - 4; 
    
        data_OK = data_OK && (GPS_sentence_string.charAt(asterisk_should_be_here) == '*');

        if(GPSECHO_GPS_query)
          {
          Serial.print("expected asterisk position "); Serial.print(asterisk_should_be_here); 
          Serial.print(" at that position: "); Serial.println(GPS_sentence_string.charAt(asterisk_should_be_here));
          }
    
        // now check that the sentence is not too short.      
        data_OK = data_OK && (GPS_sentence_string.length() >= GPSMINLENGTH);
    
        if (!data_OK) 
          {

          if (GPSECHO_GPS_query)
            {
            Serial.print("GPS sentence not good for navigation: "); Serial.println(GPS_sentence_string);
            Serial.println("I will keep trying...");
            }
            
          lcd.setCursor(0, 0);
          lcd.print("GPS navigation  ");
          lcd.setCursor(0, 1);
          lcd.print("data not present");

          return -1;        
  
          }
                
        // so far so good, so keep going...
        
        // now parse latitude 
        
        GPS_latitude_1_string = GPS_sentence_string.substring(GPRMC_latitude_1_index1, 
        GPRMC_latitude_1_index2);
        GPS_latitude_2_string = GPS_sentence_string.substring(GPRMC_latitude_2_index1, 
        GPRMC_latitude_2_index2);
        GPS_latitude_NS_string = GPS_sentence_string.substring(GPRMC_latitude_NS_index1, 
        GPRMC_latitude_NS_index2);
    
        GPS_latitude_1 = GPS_latitude_1_string.toInt();      
        GPS_latitude_2 = GPS_latitude_2_string.toInt();      
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Latitude x 100 = "); Serial.print(GPS_latitude_1); Serial.print(".");
          Serial.print(GPS_latitude_2); Serial.println(GPS_latitude_NS_string);
          }
          
        // now parse longitude 
        
        GPS_longitude_1_string = GPS_sentence_string.substring(GPRMC_longitude_1_index1, 
        GPRMC_longitude_1_index2);
        GPS_longitude_2_string = GPS_sentence_string.substring(GPRMC_longitude_2_index1, 
        GPRMC_longitude_2_index2);
        GPS_longitude_EW_string = GPS_sentence_string.substring(GPRMC_longitude_EW_index1, 
        GPRMC_longitude_EW_index2);
    
        GPS_longitude_1 = GPS_longitude_1_string.toInt();      
        GPS_longitude_2 = GPS_longitude_2_string.toInt();      
          
        if(GPSECHO_GPS_query)
          {
          Serial.print("Longitude x 100 = "); Serial.print(GPS_longitude_1); Serial.print(".");
          Serial.print(GPS_longitude_2); Serial.println(GPS_longitude_EW_string); 
          }
    
        // now parse speed and direction. we'll need to locate the 7th and 8th commas in the
        // data sentence to do this. so use the indexOf function to find them.
        // it returns -1 if string wasn't found. the number of digits is not uniquely defined 
        // so we need to find the fields based on the commas separating them from others.
        
        int comma_A_index = GPRMC_longitude_EW_index2;
        int comma_B_index = GPS_sentence_string.indexOf(",", comma_A_index + 1);
        int comma_C_index = GPS_sentence_string.indexOf(",", comma_B_index + 1);
    
        GPS_speed_knots_string = GPS_sentence_string.substring(comma_A_index + 1, comma_B_index); 
        GPS_direction_string = GPS_sentence_string.substring(comma_B_index + 1, comma_C_index); 
        
        GPS_speed_knots = GPS_speed_knots_string.toFloat();
        GPS_direction = GPS_direction_string.toFloat();
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Speed (knots) = "); Serial.println(GPS_speed_knots);
          Serial.print("Direction (degrees) = "); Serial.println(GPS_direction);
          }
          
        // now get the (UTC) date, in format DDMMYY, e.g. 080618 for 8 June 2018.
        GPS_date_string = GPS_sentence_string.substring(comma_C_index+ + 1, comma_C_index + 7);
        
        if(GPSECHO_GPS_query)
          {
          Serial.print("date, in format ddmmyy = "); Serial.println(GPS_date_string);    
          }
    
        // Write message to LCD now. It will look like this (no satellite data in this record):
        //     Sats: 4006.9539N
        //     N/A  08815.4431W
        
        lcd.setCursor(0, 0);
        lcd.print("Sats: ");
        lcd.setCursor(6, 0);
        lcd.print(GPS_latitude_1_string); lcd.print("."); lcd.print(GPS_latitude_2_string); 
        lcd.print(GPS_latitude_NS_string); 
    
        lcd.setCursor(0, 1);
        lcd.print("N/A ");
        lcd.setCursor(5, 1);
        lcd.print(GPS_longitude_1_string); lcd.print("."); lcd.print(GPS_longitude_2_string); 
        lcd.print(GPS_longitude_EW_string);

        // print a summary of the data and parsed results:
        if(GPSECHO_GPS_query)
          {
          Serial.print("GPS sentence: "); Serial.println(GPS_sentence_string);

          Serial.print("Time (UTC) = "); Serial.print(GPS_hour); Serial.print(":");
          Serial.print(GPS_minutes); Serial.print(":");
          Serial.print(GPS_seconds); Serial.print(".");
          Serial.println(GPS_milliseconds);
        
          Serial.print("Latitude x 100 = "); Serial.print(GPS_latitude_1); Serial.print(".");
          Serial.print(GPS_latitude_2); Serial.print(" "); Serial.print(GPS_latitude_NS_string);

          Serial.print("    Longitude x 100 = "); Serial.print(GPS_longitude_1); Serial.print(".");
          Serial.print(GPS_longitude_2); Serial.print(" "); Serial.println(GPS_longitude_EW_string); 

          Serial.print("Speed (knots) = "); Serial.print(GPS_speed_knots);
          Serial.print("     Direction (degrees) = "); Serial.println(GPS_direction);

          Serial.println("There is no satellite or altitude information in a GPRMC data sentence.");
              
          }
      
        // all done with this sentence, so return.
        return 0;
          
        }  // end of "if (data_OK && GPS_command.equals("$GPRMC"))" block

    //////////////////////////////////////////////////////////////////////
    /////////////////////////// GPGGA sentence ///////////////////////////
    //////////////////////////////////////////////////////////////////////
    
      if (data_OK && GPS_command.equals("$GPGGA"))
        {

        if(GPSECHO_GPS_query) 
          {
          Serial.print("\nnew GPS sentence: "); Serial.println(GPS_sentence_string);
          }
    
        // parse the time
    
        GPS_hour_string = GPS_sentence_string.substring(GPGGA_hour_index1, GPGGA_hour_index2);
        GPS_minutes_string = GPS_sentence_string.substring(GPGGA_minutes_index1, GPGGA_minutes_index2);
        GPS_seconds_string = GPS_sentence_string.substring(GPGGA_seconds_index1, GPGGA_seconds_index2);
        GPS_milliseconds_string = GPS_sentence_string.substring(GPGGA_milliseconds_index1, 
        GPGGA_milliseconds_index2);
    
        GPS_hour = GPS_hour_string.toInt();
        GPS_minutes = GPS_minutes_string.toInt();
        GPS_seconds = GPS_seconds_string.toInt();
        GPS_milliseconds = GPS_milliseconds_string.toInt();
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Time (UTC) = "); Serial.print(GPS_hour); Serial.print(":");
          Serial.print(GPS_minutes); Serial.print(":");
          Serial.print(GPS_seconds); Serial.print(".");
          Serial.println(GPS_milliseconds);
          }
    
        // now get the fix quality and number of satellites.
    
        GPS_fix_quality_string = GPS_sentence_string.substring(GPGGA_fix_quality_index1, 
        GPGGA_fix_quality_index2);
        GPS_satellites_string = GPS_sentence_string.substring(GPGGA_satellites_index1, 
        GPGGA_satellites_index2);
    
        GPS_fix_quality = GPS_fix_quality_string.toInt();      
        GPS_satellites = GPS_satellites_string.toInt();      
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("fix quality (1 for GPS, 2 for DGPS) = "); Serial.println(GPS_fix_quality);
          Serial.print("number of satellites = "); Serial.println(GPS_satellites);
          }
    
        // now see if the data are valid: we'll expect a fix, and at least three satellites.
    
        bool data_OK = (GPS_fix_quality > 0) && (GPS_satellites >= 3); 
    
        // now look for the asterisk.
        int asterisk_should_be_here = GPS_sentence_string.length() - 4; 
    
        data_OK = data_OK && (GPS_sentence_string.charAt(asterisk_should_be_here) == '*');
    
        // now check that the sentence is not too short.      
        data_OK = data_OK && (GPS_sentence_string.length() >= GPSMINLENGTH);

        if (!data_OK) 
          {
           
          if (GPSECHO_GPS_query)
            {
            Serial.print("GPS sentence not good for navigation: "); Serial.println(GPS_sentence_string);
            Serial.println("I will keep trying...");
            }
            
          lcd.setCursor(0, 0);
          lcd.print("GPS navigation  ");
          lcd.setCursor(0, 1);
          lcd.print("data not present");
          
          }
    
        // if data are not good, go back to the top of the loop by breaking out of this if block.
        
        if (!data_OK) break;
            
        // so far so good, so keep going...
        
        // now parse latitude 
        
        GPS_latitude_1_string = GPS_sentence_string.substring(GPGGA_latitude_1_index1, 
        GPGGA_latitude_1_index2);
        GPS_latitude_2_string = GPS_sentence_string.substring(GPGGA_latitude_2_index1, 
        GPGGA_latitude_2_index2);
        GPS_latitude_NS_string = GPS_sentence_string.substring(GPGGA_latitude_NS_index1, 
        GPGGA_latitude_NS_index2);
    
        GPS_latitude_1 = GPS_latitude_1_string.toInt();      
        GPS_latitude_2 = GPS_latitude_2_string.toInt();      
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Latitude x 100 = "); Serial.print(GPS_latitude_1); Serial.print(".");
          Serial.print(GPS_latitude_2); Serial.println(GPS_latitude_NS_string);
          }
          
        // now parse longitude 
        
        GPS_longitude_1_string = GPS_sentence_string.substring(GPGGA_longitude_1_index1, 
        GPGGA_longitude_1_index2);
        GPS_longitude_2_string = GPS_sentence_string.substring(GPGGA_longitude_2_index1, 
        GPGGA_longitude_2_index2);
        GPS_longitude_EW_string = GPS_sentence_string.substring(GPGGA_longitude_EW_index1, 
        GPGGA_longitude_EW_index2);
    
        GPS_longitude_1 = GPS_longitude_1_string.toInt();      
        GPS_longitude_2 = GPS_longitude_2_string.toInt();      
    
        if(GPSECHO_GPS_query)
          {         
          Serial.print("Longitude x 100 = "); Serial.print(GPS_longitude_1); Serial.print(".");
          Serial.print(GPS_longitude_2); Serial.println(GPS_longitude_EW_string); 
          }
          
        // let's skip the "horizontal dilution" figure and go straight for the altitude now.
        // this begins two fields to the right of the num,ber of satellites so find this
        // by counting commas. use the indexOf function to find them.
        int comma_A_index = GPS_sentence_string.indexOf(",", GPGGA_satellites_index2 + 1);
        int comma_B_index = GPS_sentence_string.indexOf(",", comma_A_index + 1);
    
        GPS_altitude_string = GPS_sentence_string.substring(comma_A_index + 1, comma_B_index); 
        
        GPS_altitude = GPS_altitude_string.toFloat();
    
        if(GPSECHO_GPS_query)
          {
          Serial.print("Altitude (meters) = "); Serial.println(GPS_altitude);
          }
    
        // Write message to LCD now. It will look like this:
        //     Sats: 4006.9539N
        //       10 08815.4431W
             
        lcd.setCursor(0, 0);
        lcd.print("Sats: ");
        lcd.setCursor(6, 0);
        lcd.print(GPS_latitude_1_string); lcd.print("."); lcd.print(GPS_latitude_2_string); 
        lcd.print(GPS_latitude_NS_string); 
    
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.setCursor(2, 1);
        lcd.print(GPS_satellites);
        lcd.setCursor(5, 1);
        lcd.print(GPS_longitude_1_string); lcd.print("."); lcd.print(GPS_longitude_2_string); 
        lcd.print(GPS_longitude_EW_string);

        // print a summary of the data and parsed results:
        if(GPSECHO_GPS_query)
          {
          Serial.print("GPS sentence: "); Serial.println(GPS_sentence_string);

          Serial.print("Time (UTC) = "); Serial.print(GPS_hour); Serial.print(":");
          Serial.print(GPS_minutes); Serial.print(":");
          Serial.print(GPS_seconds); Serial.print(".");
          Serial.println(GPS_milliseconds);
        
          Serial.print("Latitude x 100 = "); Serial.print(GPS_latitude_1); Serial.print(".");
          Serial.print(GPS_latitude_2); Serial.print(" "); Serial.print(GPS_latitude_NS_string);

          Serial.print("    Longitude x 100 = "); Serial.print(GPS_longitude_1); Serial.print(".");
          Serial.print(GPS_longitude_2); Serial.print(" "); Serial.println(GPS_longitude_EW_string); 

          Serial.print("Speed (knots) = "); Serial.print(GPS_speed_knots);
          Serial.print("     Direction (degrees) = "); Serial.println(GPS_direction);

          Serial.print("Number of satellites: "); Serial.print(GPS_satellites);
          Serial.print("       Altitude (meters): "); Serial.println(GPS_altitude);
              
          }
       
        // all done with this sentence, so return.
        return 0;
       
      }   // end of "if (data_OK && GPS_command.equals("$GPGGA"))" block
  
    // we'll fall through to here (instead of returning) when we've read a complete 
    // sentence, but it doesn't have navigational information (for example, an antenna 
    // status record).
    
    } 

  }



//////////////////////////////////////////////////////////////////////
////////////////////////// LCD_message function //////////////////////
//////////////////////////////////////////////////////////////////////

void LCD_message(String line1, String line2)
{
  // write two lines (of 16 characters each, maximum) to the LCD display.
  // I assume an object named "lcd" has been created already, has been 
  // initialized in setup, and is global.

  // set the cursor to the beginning of the first line, clear the line, then write.
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(line1);

  // now do the next line.
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(line2);

  return;
}

//////////////////////////////////////////////////////////////////////
////////////////////// DS3231_query function /////////////////////////
//////////////////////////////////////////////////////////////////////

void DS3231_query()
{
  // read from the DS3231 real time clock.

  // declare type for a couple of local variables
  int RTC_minute, RTC_second;
  
  // instantiate the device as an object named "now," then call its class functions.
  DateTime now = rtc.now();

  // write time information to LCD. Year, etc. first, after setting LCD to second line.
  lcd.setCursor(0, 1);
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);

  // delay a second
  delay(1000);

  // now display the day of the week and time.
  lcd.setCursor(0, 1);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(". ");
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  
  RTC_minute = now.minute();
  if (RTC_minute < 10) {lcd.print('0');}
  lcd.print(RTC_minute, DEC);
  lcd.print(':');
  
  RTC_second = now.second();
  if (RTC_second < 10) {lcd.print('0');}
  lcd.print(RTC_second, DEC);

  // there are other functions available, such as 
  //    now.unixtime();
  //    DateTime future (now + TimeSpan(7,12,30,6));
  //    future.year(); etc. etc.

}
