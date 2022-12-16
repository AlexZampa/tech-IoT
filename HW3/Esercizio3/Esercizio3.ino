#include <ArduinoJson.h>
#include <MQTTclient.h>
#include <Bridge.h>

const int LED_PIN = 8;
const int TEMP_PIN = A1;
const int B = 4275;
const float T0 = 298.15;
const String my_base_topic = "/tiot/16";
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);


void setLedValue(const String& topic, const String& subtopic, const String& message){
  DeserializationError err = deserializeJson(doc_rec, message);
  if(err){
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.c_str());
  }
  if(doc_rec["e"][0]["n"] == "led"){
    if(doc_rec["e"][0]["v"] == 0 || doc_rec["e"][0]["v"] == 1){
      digitalWrite(LED_PIN, doc_rec["e"][0]["v"]);
    }
    else{
      Serial.println("value must be 0 or 1");
    }
  }
  else{
      Serial.println("Invalid command");
  }
}

void prova(const String& topic, const String& subtopic, const String& message){
  Serial.println(topic);
  Serial.println(subtopic);
  Serial.println(message);
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
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  
  
  mqtt.begin("test.mosquitto.org", 1883);
  //mqtt.subscribe(my_base_topic + String("/led"), setLedValue);
  mqtt.subscribe(my_base_topic + String("/c/d/s/3fe07875-d1f4-47d1-ac81-0bac34593d21"), prova);
}


void loop() {
  mqtt.monitor();
  //String message = senMlEncode("temperature", get_temp(), "Celsius");
  //mqtt.publish(my_base_topic + String("/temperature"), message);
  char data[] = "{\"dID\":\"3fe07875-d1f4-47d1-ac81-0bac34593d21\",\"endp\":{\"M\":{\"temp\":{\"t\":\"/tiot/16/temp\"},\"led\":{\"t\":\"/tiot/16/led\",\"v\":[0,1]}}},\"res\":{\"m\":[\"temp\"],\"c\":[\"led\"]}}";
  mqtt.publish(my_base_topic + String("/c/d/s"), data);
  delay(1000);
}
