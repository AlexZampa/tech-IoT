#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <LiquidCrystal_PCF8574.h>
#define BUF_LENGTH 128

#define CLAP 1
/* Definendo CLAP = 1 si attiva la parte di codice del punto 9 e il led collegato al LED_PIN verrà acceso al doppio battito di mani.
 * Con CLAP = 0 il codice rimane quello precedente per il rilevamento della presenza tramite il sensore di rumore.
 */

/* PS: il nostro gruppo non ha potuto testare le parti di codice del sensore di rumore, perchè il sensore non funziona.
 * I valori delle tempistiche sono quelli di esempio riportati sul testo del laboratorio.
 */
struct temp_range_s {
  double low[2], high[2];
};

//Setup temperatura
const int TEMP_PIN = A1;        //pin sensore di temperatura
const int FAN_PIN = 9;          //pin PWM Fan 
const int HEAT_PIN = 6;         //pin LED rosso
const int B = 4275;

const double T0 = 298.15;       //25 °C -> °K
struct temp_range_s fan_temp = { {27, 25}, {35, 30}};   // posizione 0 = non presenza, posizione 1 = presenza
struct temp_range_s heat_temp = { {15, 15}, {25, 25}};  // posizione 0 = non presenza, posizione 1 = presenza

//Setup PIR
const int PIR_PIN = 8;            //pin sensore PIR
int presence_pir = 0;
unsigned long last_detect_pir = 0;
const int timeout_pir = 60 * 30 * 1e3;   //30 minuti

//Setup rumore
const int SOUND_PIN = 7;
const int n_sound_events = 50;    //50    
volatile int presence_sound = 0;                        // rilevata presenza dal sensore di rumore
volatile int recorded_sound_events = 0;                 // numero eventi rilevati dal sensore
volatile unsigned long first_record = 0;                // momento di tempo del primo evento rilevato
volatile unsigned long last_detect_sound = 0;           // momento di tempo in cui si rileva la presenza
const int sound_interval = 10 * 60 * 1e3; //10 minuti   //intervallo in cui devono avvenire n_sound_events per rilevare una presenza
const int timeout_sound = 60 * 60 * 1e3; //60 minuti    //timeout di validità della presenza

//Setup LCD
LiquidCrystal_PCF8574 lcd(0x27);
unsigned long last_switch = 0;                  // momento di tempo in cui è stato fatto l'ultimo cambio di pagina
const int timeout_switch = 5 * 1e3;             //5 secondi  timeout dopo il quale si cambia pagina
int page = 0;                                   // numero pagina attuale
const int max_page = 1;                         // l'ultima pagina visuallizabile (partendo da 0)

//Setup Clap-Clap
const int LED_PIN = 10;                        // pin LED verde
const int clap_interval = 3 * 1e3;             // 3 secondi  tempo in cui devono avvenire i due battiti di mani
const int n_clap_events = 2;                   // numero battiti di mani che attiva il LED 
volatile int clap_events = 0;                  // conteggio numero di battiti rilevati
volatile unsigned long first_clap = 0;         // momento di tempo del primo battito rilevato
volatile int led_status = 0;                   // status del LED
volatile unsigned int last_detect_clap = 0;    // momento di tempo in cui si è rilavato il doppio battito
const int timeout_clap = 60 * 60 * 1e3; //60 minuti    // timeout di validità della presenza

/* Interprete dei comandi
 * comando help per vedere quelli disponibili
 */
void interpreter() {
  int argc = 0, data, len = 0;
  static char buf[BUF_LENGTH];
  while (Serial.available()) {              // Lettura dei caratteri dalla seriale
    data = Serial.read();
    if (data == '\b' || data == '\177') {
      if (len) {
        len--;
      }
    }
    else if (data == '\r' || data == '\n' || data == ' ' || data == '\0') {
      buf[len++] = '\0';
      if (data == ' ')                    //conto gli argomenti e li separo con '\0'
        argc++;
    }
    else if (len < BUF_LENGTH - 1) {
      buf[len++] = data;
    }
  }

  /*  Sintassi comandi: set_temp mode presence end_point temp
   *  - mode = fan|heat
   *  - presence = 0|1
   *  - end_point = low|high
   *  - temp = temperature in Celsius
   */
  if (!strcmp(buf, "set_temp") && argc == 4) {     //len = 9
    if (!strcmp(buf + 9, "fan")) {                 //len = 13
      if (!strcmp(buf + 13, "0")) {                //len = 15
        if (!strcmp(buf + 15, "low")) {            //len = 19
          fan_temp.low[0] = atof(buf + 19);
        } else if (!strcmp(buf + 15, "high")) {    //len = 20
          fan_temp.high[0] = atof(buf + 20);
        } else {
          Serial.println("Wrong format");
        }
      } else if (!strcmp(buf + 13, "1")) {         //len = 15
        if (!strcmp(buf + 15, "low")) {            //len = 19
          fan_temp.low[1] = atof(buf + 19);
        } else if (!strcmp(buf + 15, "high")) {    //len = 20
          fan_temp.high[1] = atof(buf + 20);
        } else {
          Serial.println("Wrong format");
        }
      } else {
        Serial.println("Wrong format");
      }
    } else if (!strcmp(buf + 9, "heat")) {         //len = 14
      if (!strcmp(buf + 14, "0")) { //len = 16
        if (!strcmp(buf + 16, "low")) {            //len = 20
          heat_temp.low[0] = atof(buf + 20);
        } else if (!strcmp(buf + 16, "high")) {    //len = 21
          heat_temp.high[0] = atof(buf + 21);
        } else {
          Serial.println("Wrong format");
        }
      } else if (!strcmp(buf + 14, "1")) {        //len = 16
        if (!strcmp(buf + 16, "low")) {           //len = 20
          heat_temp.low[1] = atof(buf + 20);
        } else if (!strcmp(buf + 16, "high")) {   //len = 21
          heat_temp.high[1] = atof(buf + 21);
        } else {
          Serial.println("Wrong format");
        }
      } else {
        Serial.println("Wrong format");
      }
    } else {
      Serial.println("Wrong format");
    }
  } else if (!strcmp(buf, "show_temp")) {        // comando per mostrare i set-point sul monior seriale
    Serial.println("Temperature breakpoints");
    Serial.println("\tFan:");
    Serial.print("\t\tLow ");
    Serial.print(fan_temp.low[0]);
    Serial.print(" if presence ");
    Serial.println(fan_temp.low[1]);
    Serial.print("\t\tHigh ");
    Serial.print(fan_temp.high[0]);
    Serial.print(" if presence ");
    Serial.println(fan_temp.high[1]);
    Serial.println("\tHeat:");
    Serial.print("\t\tLow ");
    Serial.print(heat_temp.low[0]);
    Serial.print(" if presence ");
    Serial.println(heat_temp.low[1]);
    Serial.print("\t\tHigh ");
    Serial.print(heat_temp.high[0]);
    Serial.print(" if presence ");
    Serial.println(heat_temp.high[1]);
  } else if (!strcmp(buf, "help")) {            // Mostra i comandi accettabili
    if (argc == 1) {
      if (!strcmp(buf + 5, "set_temp")) {
        Serial.println("set_temp mode presence end_point temp\n - mode = fan|heat\n - presence = 0|1\n - end_point = low|high\n - temp = temperature in Celsius");
      } else if (!strcmp(buf + 5, "show_temp")) {
        Serial.println("show_temp\n Shows the temperature breakpoints");
      } else {
        Serial.println("Unknown command");
      }
    } else {
      Serial.println("Command help:\n - set_temp\n - show_temp");
    }
  } else {
    Serial.println("Unknown command");
  }
}


/* Legge il valore analogico e lo converte in gradi Celsius usando la formula vista al Lab1
 * 
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

#if CLAP
/* Versione per il punto 9 per la rilevazione del battito di mani
 * Conta il numero di eventi ricevuti dalla ISR e se vi sono 2 (o più) eventi
 * all'interno del lasso di tempo (2 secondi) inverte lo stato del led.
 */
void get_sound() {
  if (!clap_events) //Se è il primo
    first_clap = millis(); //Memorizzo il momento
  clap_events++; //Conto l'evento
}
#else
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
#endif


/* Controlla la velocità della ventola data la temperatura attuale e il valore di presenza o meno
 * Ritorna la percentuale di attivazione della ventola
 */
int fan_control(long temp, int presence) {
  if (temp < (long)fan_temp.low[presence]) { // Se sotto la temperatura minima => ventola spenta
    analogWrite(FAN_PIN, 0);
    return 0;
  }
  if (temp > (long)fan_temp.high[presence]) { // Se sopra la temperatura massima => ventola accesa la massimo
    analogWrite(FAN_PIN, 255);
    return 100;
  } // Altrimenti un valore linearmente proporzionato alla temperatura
  analogWrite(FAN_PIN, map(temp, (long)fan_temp.low[presence], (long)fan_temp.high[presence], 0, 255));
  return map(temp, (long)fan_temp.low[presence], (long)fan_temp.high[presence], 0, 100);
}


/* Controlla l'intensità del led data la temperatura attuale e il valore di presenza o meno
 * Ritorna la percentuale di intensità del led
 */
int heat_control(long temp, int presence) {
  if (temp < (long)heat_temp.low[presence]) { // Se sotto la temperatura minima => led al massimo
    analogWrite(HEAT_PIN, 255);
    return 100;
  }
  if (temp > (long)heat_temp.high[presence]) { // Se sopra la temeratura massima => led spento
    analogWrite(HEAT_PIN, 0);
    return 0;
  } // Altrimenti un valore linearmente proporzionato alla temperatura
  analogWrite(HEAT_PIN, map(temp, (long)heat_temp.low[presence], (long)heat_temp.high[presence], 255, 0));
  return map(temp, (long)heat_temp.low[presence], (long)heat_temp.high[presence], 100, 0); 
}


#if CLAP
/* controlla presenza versione punto 9 esaminando il numero di eventi e i timeout del sensore di rumore 
 * e del PIR. Accende il LED verde e ritorna la presenza
 */
int presence_control() {
  if (millis() - first_clap > clap_interval)        // controllo eventi nella finestra di tempo    
    clap_events = 0;                                // fuori finestra => eventi = 0
  else if (clap_events >= n_clap_events) {          // numero di eventi sufficente
    led_status = !led_status;                       //cambio stato del LED
    last_detect_clap = millis();                  // momento in cui si accende il LED per il timeout
    clap_events = 0;                              // reset numero di eventi
  }
  
  if (presence_pir && millis() - last_detect_pir >= timeout_pir) {      //controllo presenza del PIR e timeout
    presence_pir = 0;                           // nessuna presenza
    led_status = 0;                             // nessuna presenza => LED verde spento
  }  
  
  if (millis() - last_detect_clap >= timeout_clap) {      //controllo timeout battito di mani
    led_status = 0;                                  
  }
  digitalWrite(LED_PIN, led_status);                    //imposto stato del LED
  
  return presence_pir;
}
#else
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
#endif


/* Responsabile della stampa su display LCD delle due (o volendo più) pagine di informazioni
 * sullo stato del sistema
 */
void lcd_control(double temp, int presence, int fan, int heat) {
  if (millis() - last_switch >= timeout_switch) {     // Timeout per il cambio pagina
    page++;
    if (page > max_page)        // Ritormno all'inizio arrivato alla fine
      page = 0;
  }
  lcd.clear();
  lcd.home();
  switch (page) {
    case 0:                    //Prima pagina 
      lcd.print("T:");
      lcd.print(temp, 1);
      lcd.print(" Pres:");
      lcd.print(presence);
      lcd.setCursor(0,1);
      lcd.print("AC:");
      lcd.print(fan);
      lcd.print("% HT:");
      lcd.print(heat);
      lcd.print("%");
    break;
    case 1:                   //Seconda pagina
      lcd.print("AC m:");
      lcd.print(fan_temp.low[presence], 1);
      lcd.print(" M:");
      lcd.print(fan_temp.high[presence], 1);
      lcd.setCursor(0,1);
      lcd.print("HT m:");
      lcd.print(heat_temp.low[presence], 1);
      lcd.print(" M:");
      lcd.print(heat_temp.high[presence], 1);
    break;
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
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), get_sound, RISING);

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
}


void loop() {
  // Se vi sono dati presenti sulla porta seriale richiamiamo l'interprete dei comandi
  if (Serial.available())
    interpreter();
  double temp = get_temp();                       // Rilevo temperatura
  get_pir();                                      // Rilevo presenza dal PIR
  noInterrupts();                                 // Disabilito interrupts per poter manipolare le variabili utilizzate anche dall'ISR
  int presence = presence_control();              // Controllo della presenza da PIR e sensore di rumore / controllo del battito di mani
  interrupts();                                   // Riabilito interrupts
  int fan = fan_control((long)temp, presence);    // Controllo attivazione della ventola
  int heat = heat_control((long)temp, presence);  // Controllo attivazione del led di riscaldamento
  lcd_control(temp, presence, fan, heat);         // Stampa informazioni su LCD
  delay(2000);
}
