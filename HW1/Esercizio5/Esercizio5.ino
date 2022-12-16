#include <math.h>

const int TEMP_PIN = A1;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15; //25 °C -> °K

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 5");
}

void loop() {
  int a = analogRead(TEMP_PIN);
  float R = ((1023.0 / a) - 1) * R0;
  float temp = 1 / ((log(R / R0) / B) + (1 / T0));   //°K
  temp = temp - 273.15; //°C
  Serial.print("Temperatura: ");
  Serial.print(temp, 2);
  Serial.println("°C");
  delay(250);
}
