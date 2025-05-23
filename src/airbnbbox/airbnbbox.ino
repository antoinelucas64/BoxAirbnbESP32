#include <DNSServer.h>
#include "StringUtils.h"
#include <Wire.h>
#include "configsim.h"
#include "websim.h"
#include "sim.h"
#include "commands.h"
#include <esp_pthread.h>

#ifdef ESP32
#include <WiFi.h>  // il s’agit d’un ESP32
#define NOTIFICATION_CONNECTION_WIFI ARDUINO_EVENT_WIFI_AP_STACONNECTED
#else
#include <ESP8266WiFi.h>
#define NOTIFICATION_CONNECTION_WIFI WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP
#endif


ConfigSim myConfig;
char replybuffer[255];


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

int nbClient;

const byte DNS_PORT = 53;  // Capture DNS requests on port 53
//IPAddress local_IP(8,8,8,8);
IPAddress local_IP(10, 10, 10, 10);
IPAddress subnet(255, 255, 255, 0);
DNSServer dnsServer;
Sim sim(myConfig);
WebSim web(myConfig, sim);
/*TinyGsm modem(SerialAT);*/
bool openTheDoor = false;
bool doorIsOpen = false;
int timeDoorOpen;
 

void openDoor() {
  LockMutex lock(myConfig.getMutex());

  digitalWrite(LED_BLUE, HIGH);
  Serial.println("open\nHIGH");
  digitalWrite(RELAY_DOOR, DOOR_OPEN);

  openTheDoor = false;
  timeDoorOpen = millis();
  doorIsOpen = true;
}

void closeDoor() {
  doorIsOpen = false;
  Serial.println("close door");
  LockMutex lock(myConfig.getMutex());
  digitalWrite(RELAY_DOOR, DOOR_CLOSE);
  digitalWrite(LED_BLUE, LOW);
}

bool modem_init = false;


void *runSim(void *ptr) {
  while (true) {
    ((Sim *)ptr)->loop();
    delay(500);
  }
}
pthread_t thread;

void setup() {
  Serial.begin(115200);
  myConfig.init();
  {
    LockMutex lock(myConfig.getMutex());
    pinMode(RELAY_DOOR, OUTPUT);
    digitalWrite(RELAY_DOOR, DOOR_CLOSE);
    pinMode(RELAY_ELEC, OUTPUT);
    digitalWrite(RELAY_ELEC, myConfig.getPowerState());

    pinMode(LED_BLUE, OUTPUT);

    digitalWrite(LED_BLUE, HIGH);
  }




  nbClient = 0;
  Serial.print("Setting soft-AP configuration ... ");
  WiFi.softAPConfig(local_IP, local_IP, subnet);
  // WiFi.softAP("DNSServer CaptivePortal example");
  //WiFi.hostname("antoine");
  WiFi.onEvent([](WiFiEvent_t a) {
    openTheDoor = true;
  },
               NOTIFICATION_CONNECTION_WIFI);
  Serial.print("SSID ");
  Serial.print(myConfig.getSSID());
  Serial.print(" password ");
  Serial.println(myConfig.getPassword());
  WiFi.softAP(myConfig.getSSID().c_str(), myConfig.getPassword().c_str());

  dnsServer.start(DNS_PORT, "*", local_IP);

  // Serial.print("Soft-AP IP address = ");
  //  Serial.println(WiFi.softAPIP());

  web.init();

  Serial.println("Start modem");

 // sim.init();

  esp_pthread_cfg_t cfg = esp_pthread_get_default_config();
  Serial.print("default STACK size ");
  Serial.println(cfg.stack_size);
  cfg.stack_size = (32 * 1024);
  esp_pthread_set_cfg(&cfg);

  pthread_create(&thread, NULL, &runSim, &sim);


  LockMutex lock(myConfig.getMutex());
  digitalWrite(LED_BLUE, LOW);
}




long prevMillis = 0;
int interval = 1000;
boolean ledState = false;

void memoryMonitor() {
  // heap
  int free = ESP.getFreeHeap();
  int max = ESP.getMaxAllocHeap();
  int min = ESP.getMinFreeHeap();
  Serial.print("Heap min ");
  Serial.print(min);
  Serial.print(" max ");
  Serial.print(max);
  Serial.print(" free ");
  Serial.println(free);
  // psram
  free = ESP.getFreePsram();
  max = ESP.getMaxAllocPsram();
  min = ESP.getMinFreePsram();
  Serial.print("PSRAM min ");
  Serial.print(min);
  Serial.print(" max ");
  Serial.print(max);
  Serial.print(" free ");
  Serial.println(free);
  // psram
  free = ESP.getFreeSketchSpace();
  max = ESP.getSketchSize();

  Serial.print("Sketch size ");
  Serial.print(max);
  Serial.print(" free ");
  Serial.println(free);
}

void loop() {
  if (openTheDoor) openDoor();
  else if (doorIsOpen && millis() - timeDoorOpen > 10000) {
    closeDoor();
  }
  nbClient = WiFi.softAPgetStationNum();

  if (nbClient > 0) {
    dnsServer.processNextRequest();
    web.handleClient();
  } else {
    if (web.haveToReboot()) web.reboot();
    delay(500);
  }
   

// sim.loop();

  if (millis() - prevMillis > interval) {
    ledState = !ledState;
    // digitalWrite(LED_BLUE, ledState);

    prevMillis = millis();
    // memoryMonitor();
  }
}
