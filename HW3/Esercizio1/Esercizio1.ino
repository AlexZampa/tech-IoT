#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>


BridgeServer server;
const int LED_PIN = 8;
const int TEMP_PIN = A1;
const int B = 4275;
const float T0 = 298.15;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);


float get_temp() {
  float temp = 1 / (log(1023.0 / analogRead(TEMP_PIN) - 1) / B + 1 / T0); //°K
  temp = temp - 273.15; //°C
  return temp;
}


void process(BridgeClient client){
  String command = client.readStringUntil('/');
  command.trim();

  if(command == "led"){
    int val = client.parseInt();
    if(val == 0 || val == 1){
      digitalWrite(LED_PIN,val);
      printResponse(client, 200, senMlEncode(F("led"), val, F("")));
    }
    else{
      printResponse(client, 400, "The value must be 0 or 1");
    }
  }
  else if(command == "temperature"){
     printResponse(client, 200, senMlEncode(F("temperature"), get_temp(), F("Celsius")));
  }
  else{
    printResponse(client, 404, "Not found");
  }
}


void printResponse(BridgeClient client, int code, String body){
  client.println("Status: " + String(code));
  if(code == 200){
    client.println(F("Content-type: application/json; charset=utf-8"));
  }
  client.println();
  client.println(body);
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


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  BridgeClient client = server.accept();
  if(client) {
    process(client);
    client.stop();
  }
  delay(50);
}
