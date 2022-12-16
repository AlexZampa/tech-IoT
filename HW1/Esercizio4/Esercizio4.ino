const int CONTROL_PIN = 6;
const int STEP = 20;

int velocity = 0;

void serialCommand() {
  if(Serial.available()>0)
  {
    int inByte = Serial.read();
    if(inByte == '+'){
      velocity += STEP;
      if(velocity > 255){
        Serial.println("Hai raggiunto il limite massimo");
        velocity = 255;
      }
      else{
        Serial.print("Velocità: ");
        Serial.println(velocity);
      }
    }
    else if(inByte == '-'){
      velocity -= STEP;
      if(velocity < 0){
        Serial.println("Hai raggiunto il limite minimo");
        velocity = 0;
      }
      else{
        Serial.print("Velocità: ");
        Serial.println(velocity);
      }
    }
    else{
      Serial.println("Error");
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 4");
  pinMode(CONTROL_PIN,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  serialCommand();
  analogWrite(CONTROL_PIN,velocity);
}
