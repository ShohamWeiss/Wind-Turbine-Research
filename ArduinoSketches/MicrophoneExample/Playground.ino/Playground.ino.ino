#include <SPI.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  int i = 0;
  int sample;
  long sum = 0;
  long sumsq = 0;
  while ( i < 1000 )
  {
    sample = analogRead(7) - 325;
    sum += sample;
    sumsq += sample * sample;
    i++;
  }
  double ave = sum / 1000;
  double avesq = sumsq / 1000;
  double rms = sqrt(avesq - (ave * ave));
  Serial.println("Average: " + String(ave));
  Serial.println("Average of Squares: " + String(avesq));
  Serial.println("RMS: " + String(rms));
}
