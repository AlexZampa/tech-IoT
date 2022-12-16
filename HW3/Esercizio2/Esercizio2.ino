#include <Process.h>
#include <Bridge.h>
#include <ArduinoJson.h>


const int TEMP_PIN = A1;
const int B = 4275;
const float T0 = 298.15;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
const String url = "http://192.168.100.126:8080/log";
DynamicJsonDocument doc_snd(capacity);


int postRequest(String data){
  Process p;
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(data);
  p.addParameter(url);
  p.run();
  return p.exitValue();
}

String senMlEncode(String res, float v, String unit){
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  if(unit != ""){
    doc_snd["e"][0]["u"] = unit;
  }
  else{
    doc_snd["e"][0]["u"] = (char*)NULL;
  }

  doc_snd["e"][0]["n"] = res;
  doc_snd["e"][0]["v"] = v;
  doc_snd["e"][0]["t"] = millis();

  String output;
  serializeJson(doc_snd, output);
  return output;
}


float get_temp() {
  float temp = 1 / (log(1023.0 / analogRead(TEMP_PIN) - 1) / B + 1 / T0); //°K
  temp = temp - 273.15; //°C
  return temp;
}


void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 2");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  int exit_value = postRequest(senMlEncode("temperature",get_temp(), "Celsius"));
  if(exit_value){
    Serial.println("curl error");
  }
  delay(1000);
}
