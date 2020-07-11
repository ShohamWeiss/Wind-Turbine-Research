

double x = 0;
double y = 0;
double a = 0;
double b = 0;

int windSensor = A15;
float voltageMax = 2.0;
float voltageMin = 0.4;
float voltageConversionConstant = 0.004882814;
float sensorVoltage = 0;

float windSpeedMin = 0;
float windSpeedMax = 32;

int windSpeed = 0;
int prevWindSpeed = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
//   delay(100);
int sensorValue = analogRead(windSensor);

float voltage = sensorValue * (5.0 / 1023.0);

sensorVoltage = sensorValue * voltageConversionConstant;


if (sensorVoltage <= voltageMin) {
  windSpeed = 0; //Chech if voltage is below minimum value. if so, set wind speed to zero.
} else {
  windSpeed = ((sensorVoltage - voltageMin) * windSpeedMax / (voltageMax - voltageMin)) * 2.232694;
}

x = windSpeed;
if (x >= y) {
  y = x;
} else {
  y = y;
}

a = sensorVoltage;
if (a >= b) {
  b = a;
} else {
  b = b;
}
if (windSpeed != prevWindSpeed) {
  Serial.print("This is windspeed ");
  Serial.println(windSpeed);
  Serial.print("This is voltage ");
  Serial.println(sensorVoltage);
  prevWindSpeed = windSpeed;
  //delay(500);
}

}
