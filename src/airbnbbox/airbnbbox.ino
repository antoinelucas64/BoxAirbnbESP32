#include <WiFi.h>
#include <DNSServer.h>
#include "StringUtils.h"
#include <Wire.h>
#include "config.h"
#include "web.h"
#include "sim.h"
#include "commands.h"

#define NOTIFICATION_CONNECTION_WIFI SYSTEM_EVENT_AP_STACONNECTED

Config myConfig;
char replybuffer[255];


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

int nbClient;

const byte DNS_PORT = 53;  // Capture DNS requests on port 53
//IPAddress local_IP(8,8,8,8);
IPAddress local_IP(10, 10, 10, 10);
IPAddress subnet(255, 255, 255, 0);
DNSServer dnsServer;
Web web(myConfig);
Sim sim(myConfig);
/*TinyGsm modem(SerialAT);*/
bool openTheDoor = false;

void openDoor() {
  digitalWrite(LED_BLUE, HIGH);
  Serial.println("open\nHIGH");
  digitalWrite(RELAY_DOOR, DOOR_OPEN);
  delay(10000);
  digitalWrite(RELAY_DOOR, DOOR_CLOSE);
  digitalWrite(LED_BLUE, LOW);
  openTheDoor = false;
}

bool modem_init = false;

void setup() {
  Serial.begin(115200);
  myConfig.init();
  pinMode(RELAY_DOOR, OUTPUT);
  digitalWrite(RELAY_DOOR, DOOR_CLOSE);
  pinMode(RELAY_ELEC, OUTPUT);
  digitalWrite(RELAY_ELEC, myConfig.getPowerState());

  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_BLUE, HIGH);



  nbClient = 0;
  Serial.print("Setting soft-AP configuration ... ");
  WiFi.softAPConfig(local_IP, local_IP, subnet);
  // WiFi.softAP("DNSServer CaptivePortal example");
  //WiFi.hostname("antoine");
  WiFi.onEvent([](WiFiEvent_t a) {
    openTheDoor = true;
  },
               NOTIFICATION_CONNECTION_WIFI);
  WiFi.softAP(myConfig.getSSID().c_str(), myConfig.getPassword().c_str());

  dnsServer.start(DNS_PORT, "*", local_IP);

  // Serial.print("Soft-AP IP address = ");
  //  Serial.println(WiFi.softAPIP());

  web.init();

  Serial.println("Start modem");
  sim.init();
  digitalWrite(LED_BLUE, LOW);
}

void printWEATHERtime() {

 
// printf("free mem: %d\n",heap_caps_get_free_size(0) );
 // heap_caps_print_heap_info(0) ;
}




long prevMillis = 0;
int interval = 1000;
boolean ledState = false;
Preferences pp;
int count = 0;
void loop() {
  if (openTheDoor) openDoor();
  nbClient = WiFi.softAPgetStationNum();

  if (nbClient > 0) {
    dnsServer.processNextRequest();
    web.handleClient();
  } else delay(500);

  if (millis() - prevMillis > interval) {
    ledState = !ledState;
    //digitalWrite(LED_BLUE, ledState);

    prevMillis = millis();
  }

  sim.loop();

  count++;
  if (count % 100 == 0) {
    printWEATHERtime();
  }
}
