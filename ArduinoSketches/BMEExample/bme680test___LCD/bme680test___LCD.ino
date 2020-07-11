/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <LiquidCrystal.h>
const int rs = 12, en = 11, data4 = 36, data5 = 34, data6 = 32, data7 = 30;
LiquidCrystal lcd(rs, en, data4, data5, data6, data7);
int lines_written;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));
  lcd.begin(16, 2);
  Serial.println("About to write to LCD.");
  Serial.println("LCD lines are 16 characters long.");
  lines_written = 0;



  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  int column, row;
  lines_written += 1;
  column = 0;
  row = 0;
  lcd.setCursor(column, row);
  lcd.print("Temperature = ");
  lcd.print(bme.temperature);
  row = 1;
  lcd.setCursor(column, row);
  lcd.print("Pressure = ");
  lcd.print(bme.pressure / 100.0);
  delay(2000);
  row = 0;
  lcd.setCursor(column, row);
  lcd.print("                ");
  row = 1;
  lcd.setCursor(column, row);
  lcd.print("                ");
  delay(100);
  row = 0;
  lcd.setCursor(column, row);
  lcd.print("Humidity = ");
  lcd.print(bme.humidity);
  row = 1;
  lcd.setCursor(column, row);
  lcd.print("Gas = ");
  lcd.print(bme.gas_resistance / 1000.0);
  delay(2000);
  row = 0;
  lcd.setCursor(column, row);
  lcd.print("                ");
  row = 1;
  lcd.setCursor(column, row);
  lcd.print("                ");
  delay(100);
  row = 0;
  lcd.setCursor(column, row);
  lcd.print("Apx.Alt = ");
  lcd.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  lcd.print(" m");
  row = 1;
  lcd.setCursor(column, row);
  lcd.print("                ");
  
  Serial.println();
  delay(2000);
}
