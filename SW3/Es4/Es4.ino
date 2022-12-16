#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <LiquidCrystal_PCF8574.h>
#include <Process.h>
#include <Bridge.h>
#include <MQTTclient.h>
#include <ArduinoJson.h>

#define BUF_LENGTH 128


/* PS: il nostro gruppo non ha potuto testare le parti di codice del sensore di rumore, perchè il sensore non funziona.
 * I valori delle tempistiche sono quelli di esempio riportati sul testo del laboratorio.
 */


const char data[] = "{\"dID\":\"3fe07875-d1f4-47d1-ac81-0bac34593d21\",\"e\":{\"d\":\"/tiot/16/data\",\"c\":\"/tiot/16/com\"},\"r\":{\"m\":\"d\",\"c\":\"c\"}}";
const int capacity_rcv = JSON_OBJECT_SIZE(4) + 100;
const String deviceID = "3fe07875-d1f4-47d1-ac81-0bac34593d21";
const String url_base = "192.168.100.126:8080/";
char url_subscribe[30];
unsigned long last_update = 0;
DynamicJsonDocument doc_rcv(capacity_rcv);
char rcv[250];

//Setup temperatura
const int TEMP_PIN = A1;    //pin sensore di temperatura
const int FAN_PIN = 9;      //pin PWM Fan 
const int HEAT_PIN = 6;     //pin LED rosso
const int B = 4275;


int fan_speed = 0;
int led_status = 0;
char message_lcd[20];

const double T0 = 298.15;   //25 °C -> °K

//Setup PIR
const int PIR_PIN = 8;      //pin sensore PIR
int presence_pir = 0;
unsigned long last_detect_pir = 0;
const int timeout_pir = 60 * 30 * 1e3;   //30 minuti


//Setup rumore
const int SOUND_PIN = 7;
const int n_sound_events = 50;    //50    
volatile int presence_sound = 0;                    // rilevata presenza dal sensore di rumore
volatile int recorded_sound_events = 0;             // numero eventi rilevati dal sensore
volatile unsigned long first_record = 0;           // momento di tempo del primo evento rilevato
volatile unsigned long last_detect_sound = 0;      // momento di tempo in cui si rileva la presenza
const int sound_interval = 10 * 60 * 1e3; //10 minuti   //intervallo in cui devono avvenire n_sound_events per rilevare una presenza
const int timeout_sound = 60 * 60 * 1e3; //60 minuti    //timeout di validità della presenza


//Setup LCD
LiquidCrystal_PCF8574 lcd(0x27);
unsigned long last_switch = 0;                  // momento di tempo in cui è stato fatto l'ultimo cambio di pagina
const int timeout_switch = 5 * 1e3;     //5 secondi  timeout dopo il quale si cambia pagina
int page = 0;                             // numero pagina attuale
const int max_page = 1;                   // l'ultima pagina visuallizabile (partendo da 0)



/* Legge il valore analogico e lo converte in gradi Celsius usando la formula vista al Lab1
 * Stampa anche su monitor seriale.
 */
double get_temp() {
  double temp = 1 / (log(1023.0 / analogRead(TEMP_PIN) - 1) / B + 1 / T0); //°K
  temp = temp - 273.15; //°C
  return temp;
}

/* Rileva la presenza tramite lettura del sensore PIR 
 * e salva i millisecondi al momento della rilevazione
 */
void get_pir() {
  if (digitalRead(PIR_PIN)) {
    presence_pir = 1;
    last_detect_pir = millis();
  }
}


/* Versione per il punto 4 per la rilevazione della presenza
 * Conta il numero di eventi ricevuti dalla ISR e conferma la presenza 
 * se rileva più di n_soud_events nel lasso di tempo specificato.
 * Inoltre salva i millisecondi al momento della rilevazione della presenza
 * per il timeout (il periodo di validità della rilevazione) gestito in presence_control
 */
void get_sound() {
  if (!recorded_sound_events) //Se è il primo
    first_record = millis(); //Memorizzo il momento
  recorded_sound_events++; //Conto l'evento
}


/* controlla presenza versione punto 4 esaminando il numero di eventi e i timeout del sensore di rumore 
 * e del PIR. Ritorna la presenza
 */
int presence_control() {
  if (presence_pir && millis() - last_detect_pir >= timeout_pir) {      //controllo presenza del PIR e timeout
    presence_pir = 0;
  }
  
  if (millis() - first_record > sound_interval)     // controllo eventi nella finestra di tempo    
    recorded_sound_events = 0;
  else if (recorded_sound_events >= n_sound_events) {     // numero di eventi sufficente
    presence_sound = 1;
    last_detect_sound = millis();                       // momento in cui si rileva la presenza per il timeout
    recorded_sound_events = 0;                          // reset numero di eventi
  }
  
  if (presence_sound && millis() - last_detect_sound >= timeout_sound) {    //controllo presenza del sensore di rumore e timeout
    presence_sound = 0;
  }
  return presence_pir || presence_sound;            // basta uno dei due sensori per poter rilavare presenza
}


/* Responsabile della stampa su display LCD delle due (o volendo più) pagine di informazioni
 * sullo stato del sistema
 */
void lcd_control(double temp, int presence) {
  if (millis() - last_switch >= timeout_switch) {     // Timeout per il cambio pagina
    page++;
    if (page > max_page)        // Ritormno all'inizio arrivato alla fine
      page = 0;
  }
  lcd.clear();
  lcd.home();
  switch (page) {
    case 0: //Prima pagina 
      lcd.print("T:");
      lcd.print(temp, 1);
      lcd.print(" Pres:");
      lcd.print(presence);
      lcd.setCursor(0,1);
      lcd.print("AC:");
      lcd.print(fan_speed);
      lcd.print("% HT:");
      lcd.print(led_status);
      lcd.print("%");
    break;
    case 1: //Seconda pagina
      lcd.print(message_lcd);
      break;
  }
}


/********************************* POST REQUEST  ****************************/

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



/************************* GET REQUEST  ***************************/

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


/*************************** HANDLE COMMAND *****************************************/

void handleCommand(const String& topic, const String& subtopic, const String& message) {
  doc_rcv.clear();
  DeserializationError err = deserializeJson(doc_rcv, message);
  if(err){
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.f_str());
    return;
  }
  
  fan_speed = doc_rcv["e"][0]["v"];
  led_status = doc_rcv["e"][1]["v"];
  const char* support = doc_rcv["e"][2]["v"];
  strcpy(message_lcd, support); 
}



void fan_heat_control(){
    analogWrite(FAN_PIN, map(fan_speed, 0, 100, 0, 255));  
    analogWrite(HEAT_PIN, map(led_status, 0, 100, 0, 255));  
}



String senMlEncode(float temp, int pir, int sound){
  doc_rcv.clear();
  doc_rcv["bn"] = "Yun";
  doc_rcv["bt"] = millis();
  
  doc_rcv["e"][0]["n"] = "temp";
  doc_rcv["e"][0]["v"] = temp;
  doc_rcv["e"][0]["u"] = "Celsius";
  
  doc_rcv["e"][1]["n"] = "ppir";
  doc_rcv["e"][1]["v"] = pir;
  
  doc_rcv["e"][2]["n"] = "spir";
  doc_rcv["e"][2]["v"] = sound;
  
  String output;
  serializeJson(doc_rcv, output);
  return output;
}



/***************************** SETUP CATALOG ********************/

void setup_catalog() {
  int exit_value = getRequest(url_base, rcv);
  Serial.println(rcv);
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
      const char* support = doc_rcv["R"]["d"];
      const char* broker = doc_rcv["M"]["d"]["h"];
      strcpy(url_subscribe, support);
      
      exit_value = postRequest(url_subscribe, data, rcv);
      Serial.println(rcv);
      if (!exit_value)
      {
        doc_rcv.clear();
        error = deserializeJson(doc_rcv, rcv);
        if(error)
        {
          Serial.print("Error ");
          Serial.println(error.f_str());
        }
        else
        {
          if(!strcmp(doc_rcv["reg"], "ok"))
          {
            Serial.println(F("Succesfully registered"));
            last_update = millis();
  
            mqtt.begin(broker, 1883);
            Serial.println(F("connected mqtt"));
            mqtt.subscribe("/tiot/16/com", handleCommand);
            last_update = millis();
          }
          else
          {
            Serial.println(F("Error: not registered"));
          }
        } 
      }
    }  
  }
}


void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("SmartHomeController");
  
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), get_sound, RISING);

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  setup_catalog();
}


void loop() {
  mqtt.monitor();
  double temp = get_temp();                  // Rilevo temperatura
  get_pir();                                 // Rilevo presenza dal PIR
  noInterrupts();                            // Disabilito interrupts per poter manipolare le variabili utilizzate anche dall'ISR
  int presence = presence_control();         // Controllo della presenza da PIR e sensore di rumore / controllo del battito di mani
  interrupts();                              // Riabilito interrupts
  
  fan_heat_control();
  lcd_control(temp, presence);               // Stampa informazioni su LCD
  
  String message = senMlEncode(temp, presence_pir, presence_sound);
  mqtt.publish("/tiot/16/data", message);
  
  delay(2000);
}
