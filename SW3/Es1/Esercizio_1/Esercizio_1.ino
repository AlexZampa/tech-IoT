#include <ArduinoJson.h>
#include <MQTTclient.h>
#include <Bridge.h>
#include <Process.h>

const char data[] = "{\"dID\":\"3fe07875-d1f4-47d1-ac81-0bac34593d21\",\"e\":{\"t\":{\"t\":\"/tiot/16/temp\"},\"l\":{\"t\":\"/tiot/16/led\"}},\"r\":{\"m\":\"t\",\"c\":\"l\"}}";
const int LED_PIN = 8;
const int TEMP_PIN = A1;
const int B = 4275;
const float T0 = 298.15;

const String my_base_topic = "/tiot/16";
const int capacity_rcv = JSON_OBJECT_SIZE(4) + 100;
const String url_base = "192.168.100.126:8080/";
char topic_subscription[20];
const String deviceID = "3fe07875-d1f4-47d1-ac81-0bac34593d21";

const int period = 60 * 1e03;  //millis
unsigned long last_update = 0;

DynamicJsonDocument doc_rcv(capacity_rcv);
char rcv[220];


void setLedValue(const String& topic, const String& subtopic, const String& message){
  doc_rcv.clear();
  DeserializationError err = deserializeJson(doc_rcv, message);
  
  if(err){
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.c_str());
  }
  if(doc_rcv["e"][0]["n"] == "led"){
    if(doc_rcv["e"][0]["v"] == 0 || doc_rcv["e"][0]["v"] == 1){
      digitalWrite(LED_PIN, doc_rcv["e"][0]["v"]);
    }
    else{
      Serial.println("value must be 0 or 1");
    }
  }
  else{
      Serial.println("Invalid command");
  }
}



void update_subsciption() {
  mqtt.publish(topic_subscription, data);
}



String senMlEncode(String res, float v, String unit){
  doc_rcv.clear();
  doc_rcv["bn"] = "Yun";
  if(unit != ""){
    doc_rcv["e"][0]["u"] = unit;
  }
  else{
    doc_rcv["e"][0]["u"] = (char*)NULL;
  }

  doc_rcv["e"][0]["n"] = res;
  doc_rcv["e"][0]["v"] = v;
  doc_rcv["e"][0]["t"] = millis();

  String output;
  serializeJson(doc_rcv, output);
  return output;
}


int getRequest(String url, char* ret){
  Process p;
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Accept: application/json");
  p.addParameter("-X");
  p.addParameter("GET");
  p.addParameter(url);
  p.run();
  int i = 0;
  while(p.available() > 0){
    ret[i] = char(p.read());
    i++;
  }
  ret[i] = '\0';
  return p.exitValue();
}


void handleSubscriptionResponse(const String& topic, const String& subtopic, const String& message) {
  doc_rcv.clear();
  DeserializationError err = deserializeJson(doc_rcv, message);
 
  if(err){
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.f_str());
  }
  if(doc_rcv["reg"] == "OK")
    Serial.println(F("Registration OK"));
}


/**************************  SETUP CATALOG  ****************************************/

void setup_catalog() {
  int exit_value = getRequest(url_base, rcv);
  Serial.println(F("data from get received "));
  if (!exit_value) {
    doc_rcv.clear();
    DeserializationError error = deserializeJson(doc_rcv, rcv);
    if(error)
    {
      Serial.print(F("Error 1: "));
      Serial.println(error.f_str());
    }
    else
    {
      const char* support = doc_rcv["M"]["d"]["h"] ;
      const char* topic = doc_rcv["M"]["d"]["t"];
      strcpy(topic_subscription,topic);
    
      mqtt.begin(support, 1883);
      Serial.println(F("connected mqtt"));
      
      mqtt.subscribe(String(topic_subscription) + String("/") + deviceID, handleSubscriptionResponse);
      mqtt.publish(topic_subscription, data);
      last_update = millis();
    }  
  }
}


float get_temp() {
  float temp = 1 / (log(1023.0 / analogRead(TEMP_PIN) - 1) / B + 1 / T0);   //°K
  temp = temp - 273.15; //°C
  return temp;
}



void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println(F("Esercizio 1"));
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  
  setup_catalog();
  mqtt.subscribe(my_base_topic + String("/led"), setLedValue);
}


void loop() {
  mqtt.monitor();
  if(millis() - last_update >= period)
  {
    update_subsciption();
    last_update = millis();
  }
  String message = senMlEncode("temperature", get_temp(), "Celsius");
  mqtt.publish(my_base_topic + String("/temp"), message);
  delay(1000);
}
