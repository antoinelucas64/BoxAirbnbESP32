#include <ESP8266WiFi.h>  // mettre #include <WiFi.h> s’il s’agit d’un ESP32

// GPIO utilisés, sur le NodeMCU, le GPIO 2 allume une LED bleue
#define LED_BLUE 2
#define RELAY_DOOR 4   

// Variables globales
String ssid = " Revue Hackable";
String password = "DiamondsAreForever";
int nbClient;
IPAddress local_IP(10,10,10,10);
IPAddress subnet(255,255,255,0);

void setup() {
  nbClient = 0;
  Serial.begin(115200);    // Ouvre le port COM et fixe de débit de communication
  pinMode(RELAY_DOOR,OUTPUT); 
  pinMode(LED_BLUE,OUTPUT);
  digitalWrite(LED_BLUE,LOW); // LOW = 0 : la led est allumée
  digitalWrite(RELAY_DOOR,LOW); 
  Serial.print("Start WiFi ... ");
  WiFi.softAPConfig(local_IP, local_IP, subnet); // Création du point d’accès Wi-Fi
  WiFi.softAP(ssid,password) ;
  delay(500); // pour voir la led allumée au démarrage
  digitalWrite(LED_BLUE,HIGH); // HIGH = 1 : la led est éteinte
}

void openDoor() {
    digitalWrite(LED_BLUE, LOW);
    Serial.println("Ouverture de la porte, 10 sec.\n");
    digitalWrite(RELAY_DOOR, HIGH); // On allume la led, c’est pour faire joli.
    delay(10000); // 10 secondes
    digitalWrite(RELAY_DOOR, LOW); // fermeture de la porte (de la serrure en tout cas)
    digitalWrite(LED_BLUE, HIGH);
}


void loop() {
  int current = WiFi.softAPgetStationNum();
  if( current > nbClient ) openDoor(); // une connexion Wi-FI supplémentaire: ouverture de la porte.
  nbClient = current;
  delay(500);  // attente 500 ms. 
}