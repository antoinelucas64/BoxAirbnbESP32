// ligne a commenter en cas d'un ESP8266
#define ESP32 1 

#ifdef ESP32
#include <WiFi.h>  // il s’agit d’un ESP32
#define NOTIFICATION_CONNECTION_WIFI SYSTEM_EVENT_AP_STACONNECTED
#else
#include <ESP8266WiFi.h>
#define NOTIFICATION_CONNECTION_WIFI WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP
#endif

#include "Adafruit_FONA.h"
#include <string>
#include <cstring>
#include <Preferences.h>

// GPIO la led bleue est sur le 13 sur cette carte
#define LED_BLUE 13
#define RELAY_DOOR 2
#define RELAY_ELEC 12
#define SIM800L_PWRKEY 4
// le modem SIM800L est lui-même alimenté par un GPIO
#define SIM800L_POWER 23
// Les connections entre l’ESP32 et SIM800L
#define SIM800L_RX 27
#define SIM800L_TX 26

// Variables globales
String ssid = "Revue Hackable";
String password = "DiamondsAreForever";
int power = HIGH;
int nbClient;
IPAddress local_IP(10, 10, 10, 10);
IPAddress subnet(255, 255, 255, 0);

Adafruit_FONA sim800l(SIM800L_PWRKEY);
HardwareSerial* sim800lSerial = &Serial1;


Preferences config;
bool openTheDoor = false;
// à appeler dans le setupWifi, avant la configuration du Wi-Fi
void readConfig() {
  config.begin("config", true);
  password = config.getString("password", password);
  power = config.getInt("power", power);
  Serial.println("Read password [" + password + "]");
  config.end();
}


// à appeler dans le sendSMS
void writeConfig() {
  config.begin("config", false);
  config.putString("password", password);
  config.putInt("power", power);
  config.end();
}

void setupWifi() {
  nbClient = 0;
  Serial.begin(115200);  // Ouvre le port COM et fixe de débit de communication
  pinMode(RELAY_DOOR, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);  // LOW = 0 : la led est allumée
  digitalWrite(RELAY_DOOR, LOW);
  Serial.print("Start WiFi ... ");
  readConfig();
  WiFi.softAPConfig(local_IP, local_IP, subnet);  // Création du point d’accès Wi-Fi
  WiFi.softAP(ssid.c_str(), password.c_str());
  delay(500);                    // pour voir la led allumée au démarrage
  digitalWrite(LED_BLUE, HIGH);  // HIGH = 1 : la led est éteinte


}
void openDoor() {

//void openDoor(WiFiEvent_t a ,WiFiEventInfo_t b) {
  digitalWrite(LED_BLUE, LOW);
  Serial.println("Ouverture de la porte, 10 sec.\n");
  digitalWrite(RELAY_DOOR, HIGH);  // On allume la led, c’est pour faire joli.
  delay(10000);                    // 10 secondes
  digitalWrite(RELAY_DOOR, LOW);   // fermeture de la porte (de la serrure en tout cas)
  digitalWrite(LED_BLUE, HIGH);
  openTheDoor = false;
}

void setup() {
  pinMode(RELAY_ELEC, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);
  digitalWrite(RELAY_ELEC, power);
  // on allume le model Sim800
  digitalWrite(SIM800L_POWER, HIGH);

  setupWifi();  // le code pour initialiser le WiFi
  delay(6000);

  // Connection entre l’ESP32 et le modem SIM800L
  sim800lSerial->begin(4800, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  while (!sim800l.begin(*sim800lSerial)) {
    Serial.println("Couldn't find GSM SIM800L");
    sleep(100);
  }
  // une commande un peu magique qui semble nécessaire...
  sim800lSerial->print("AT+CNMI=2,1\r\n");
  Serial.println("GSM SIM800L Ready");
  WiFi.onEvent([](WiFiEvent_t a) { openTheDoor = true;},NOTIFICATION_CONNECTION_WIFI);

}


void loopWifi() {
  if(openTheDoor) openDoor();
  delay(100);  // attente 100 ms.
}

void loop() {
  loopWifi();
  char buffer[250];
  if (sim800l.available()) {
    int slot = 0;  // ce devrait être le numéro du SMS dans la carte SIM800
    int charCount = 0;
    // Lecture du flux text
    for (int i = 0; i < sizeof(buffer) - 1; i++) {
      buffer[i] = sim800l.read();
      if (buffer[i] == '\n') {
        buffer[i + 1] = 0;
        break;
      }
    }
    //Scan the notification string for an SMS received notification.
    if (1 == sscanf(buffer, "+CMTI: \"SM\",%d", &slot)) {
      digitalWrite(LED_BLUE, HIGH);
      String smsString = "";
      char phone[32];
      // Retrieve SMS sender address/phone number.
      if (!sim800l.getSMSSender(slot, phone, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print("FROM = ");
      Serial.println(phone);

      // Retrieve SMS value.
      uint16_t smslen;
      // Lecture du message
      if (sim800l.readSMS(slot, buffer, 250, &smslen)) {
        smsString = String(buffer);
        Serial.println(smsString);
      }
      smsString.toUpperCase();
      if (strstr(smsString.c_str(), "PASSWORD") != NULL) {
        std::string copySms(buffer);
        copySms.erase(0, copySms.find(" ") + 1);  // supression du premier mot dans le message
        String newPassword = String(copySms.c_str());
        if (WiFi.softAP(ssid.c_str(), newPassword.c_str())) {
          password = newPassword;
        } else {
          Serial.println("incorrect password ");
        }
        sendSMS(phone);
      } else if (strstr(smsString.c_str(), "ON") != NULL) {
        power = HIGH;
        digitalWrite(RELAY_ELEC, power);
        sendSMS(phone);
      } else if (strstr(smsString.c_str(), "OFF") != NULL) {
        power = LOW;
        digitalWrite(RELAY_ELEC, power);
        sendSMS(phone);
      } else {
        Serial.println("message ignore");
      }
      // cela devrait supprimer le message
      sim800l.deleteSMS(slot);
      digitalWrite(LED_BLUE, LOW);
    }
  }
}
void sendSMS(const char* telephone) {
  String msg = "Power is ";
  if (digitalRead(RELAY_ELEC) == HIGH) msg += "ON ";
  else msg += "OFF ";
  msg += "password is [" + password + "]";
  delay(100);
  sim800l.sendSMS(telephone, msg.c_str());
  delay(100);
  writeConfig();
}
