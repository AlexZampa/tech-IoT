#include <TimerOne.h>

const int RLED_PIN = 12;
const int GLED_PIN = 11;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

int greenLedState = LOW;
int redLedState = LOW;

void serialPrintStatus() {
  if(Serial.available()>0)
  {
    int inByte = Serial.read();
  
    if(inByte == 'R'){
      Serial.print("Led rosso: stato ");
      Serial.println(redLedState);
    }
    else if(inByte == 'G'){
      Serial.print("Led verde: stato ");
      Serial.println(greenLedState);
    }
    else{
      Serial.println("Error");
    }
  }
}


void blinkGreen(){
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 2");
  
  pinMode(RLED_PIN,OUTPUT);
  pinMode(GLED_PIN,OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);
  Timer1.attachInterrupt(blinkGreen);
}

void loop() {
  // put your main code here, to run repeatedly:
  serialPrintStatus();
  redLedState = !redLedState;  
  digitalWrite(RLED_PIN, redLedState);  
  delay(R_HALF_PERIOD * 1e03);
}
