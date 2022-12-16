#include <math.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x27);
 
const int TEMP_PIN = A1;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15; //25 °C -> °K

void setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("Esercizio 6");
  delay(3000);
}

void loop() {
  int a = analogRead(TEMP_PIN);
  float R = ((1023.0 / a) - 1) * R0;
  float temp = 1 / ((log(R / R0) / B) + (1 / T0));   //°K
  temp = temp - 273.15; //°C
  lcd.clear();
  lcd.print("Temperature:");
  lcd.print(temp, 1);
  delay(250);
}
