#include <Process.h>
#include <Bridge.h>
#include <ArduinoJson.h>

const int TEMP_PIN = A1;
const int B = 4275;
const float T0 = 298.15;
const int capacity_snd = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
const int capacity_rcv = JSON_OBJECT_SIZE(4) + 100;
const String url_base = "http://192.168.100.126:8080/";
String url_subscribe = "";
String deviceID = "";
char rcv[340];
const int period = 60 * 1e03;  //millis
unsigned long last_update = 0;

DynamicJsonDocument doc_snd(capacity_snd);
DynamicJsonDocument doc_rcv(capacity_rcv);


int postRequest(String url, String data, char* ret){
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
  int i = 0;
  while(p.available() > 0) {
    ret[i] = char(p.read());
    i++;
  }
  ret[i] = '\0';
  return p.exitValue();
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
  while(p.available() > 0) {
    ret[i] = char(p.read());
    i++;
  }
  ret[i] = '\0';
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


void update_subsciption() {
  int exit_value = postRequest(url_subscribe + "/" + deviceID, "", rcv);
  Serial.println("update request");
  if (exit_value) {
    Serial.println("curl error");
  }
}


void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Esercizio 4");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);

  
  int exit_value = getRequest(url_base, rcv);
  if (!exit_value) {
    DeserializationError error = deserializeJson(doc_rcv, rcv);
    if(error)
    {
      Serial.print("Errore ");
      Serial.println(error.f_str());
    }
    else
    {
      url_subscribe = doc_rcv["subscriptions"]["REST"]["device"].as<String>();
      
      exit_value = postRequest(url_subscribe, "{\"end-points\": [ \"/temperature\" ], \"resources\": [ \"Temperature\" ] }", rcv);
      Serial.print("post request ");
      Serial.println(exit_value);
      Serial.println(rcv);
      if (!exit_value)
      {
        error = deserializeJson(doc_rcv, rcv);
        if(error)
        {
          Serial.print("Errore ");
          Serial.println(error.f_str());
        }
        else{
          deviceID = doc_rcv["deviceID"].as<String>();
          last_update = millis();
        } 
      }
    }  
  }
  doc_rcv.clear();
}


void loop() { 
   if(millis() - last_update >= period)
   {
    update_subsciption();
    last_update = millis();
   }
  delay(1000);
}
