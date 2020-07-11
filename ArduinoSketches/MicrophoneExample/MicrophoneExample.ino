/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
 
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

unsigned long startProgram; //keep track of the start of the program

int counts = 0;
double sum = 0;
double sumOfSquares = 0;
double ave = 0;
double rms = 0;

 
void setup() 
{
   Serial.begin(9600);
   startProgram= millis();  // Start of the program
}
 
 
void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(7);
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
   double volts = peakToPeak;  // convert to volts
   sum += volts;
   sumOfSquares += volts * volts;
   counts += 1;
   if (millis() - startProgram > 1000) // calculate average and rms every second
   {
      ave = sum / counts;
      rms = sqrt(sumOfSquares / counts);
      sum = 0;
      sumOfSquares = 0;
      counts = 0;
      startProgram= millis();  // restart count of the program
   }
   Serial.print("Volts:"); Serial.print(volts); Serial.print(",");
   Serial.print("Average:"); Serial.print(ave); Serial.print(",");
   Serial.print("RMS:"); Serial.println(rms);
}
