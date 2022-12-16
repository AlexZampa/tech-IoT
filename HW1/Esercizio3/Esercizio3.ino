#include <TimerOne.h>

const int RLED_PIN = 6;
const int PIR_PIN = 7;
const int SEC = 3;

volatile int tot_count = 0;

void checkPresence(){
  tot_count++;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 3");
  
  pinMode(RLED_PIN,OUTPUT);
  pinMode(PIR_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  noInterrupts();
  Serial.println(tot_count); 
  interrupts();
  
  delay(SEC * 1e03);
}
