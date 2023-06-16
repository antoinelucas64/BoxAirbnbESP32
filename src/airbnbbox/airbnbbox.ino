#include <WiFi.h>
#include <DNSServer.h>
#include "StringUtils.h"
#include <Wire.h>
#include "config.h"
#include "web.h"
#include "sim.h"
#include "commands.h"


Config myConfig;
char replybuffer[255];


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

int nbClient;

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
//IPAddress local_IP(8,8,8,8);
IPAddress local_IP(10,10,10,10);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
DNSServer dnsServer;
Web web(myConfig);
Sim sim(myConfig);
/*TinyGsm modem(SerialAT);*/

void checkDoor(){
 int current = WiFi.softAPgetStationNum();
  if( current > nbClient ) {
     openDoor();
  }

  nbClient = current;
  
  //Serial.printf("Stations connected to soft-AP = %d\r\n", WiFi.softAPgetStationNum());
  //Serial.println("");

}

bool modem_init = false;

void setup() {
  Serial.begin(115200);
  myConfig.init();
  pinMode(RELAY_DOOR,OUTPUT);
  digitalWrite(RELAY_DOOR,DOOR_CLOSE);
  pinMode(RELAY_ELEC,OUTPUT);
  digitalWrite(RELAY_ELEC,myConfig.getPowerState());

  pinMode(LED_BLUE, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);

  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(SIM800L_POWER, HIGH);

  
  
  nbClient = 0;
  Serial.print("Setting soft-AP configuration ... ");
  WiFi.softAPConfig(local_IP, local_IP, subnet) ;
   // WiFi.softAP("DNSServer CaptivePortal example");
//WiFi.hostname("antoine");
  WiFi.softAP(myConfig.getSSID(),myConfig.getPassword()) ;
  
  dnsServer.start(DNS_PORT, "*", local_IP);

 // Serial.print("Soft-AP IP address = ");
//  Serial.println(WiFi.softAPIP());

  web.init();

  Serial.println("Start modem");
  delay(6000);
  sim.init();
  digitalWrite(LED_BLUE, LOW);

}

long prevMillis = 0;
int interval = 1000;
boolean ledState = false;
  Preferences pp;

void loop() {
  checkDoor();
  if(nbClient > 0) {
    dnsServer.processNextRequest();
    web.handleClient();
  }
  else delay(500);

 if (millis() - prevMillis > interval) {
    ledState = !ledState;
    //digitalWrite(LED_BLUE, ledState);

    prevMillis = millis();
  }
  
  sim.loop();
 }
