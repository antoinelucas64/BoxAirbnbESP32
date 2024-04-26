#ifndef WEB_A_A
#define WEB_A_A

#include "config.h"
#ifdef ESP32
#include <WebServer.h>
#else
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#endif

class Web {
public:
  Web(Config& config);
  ~Web(){};

  virtual void reboot();
  virtual void init();
  void handleClient();
  bool haveToReboot();

protected:

#ifdef ESP32
  WebServer server;
#else
  ESP8266WebServer server;
#endif


  bool admin = false;
  bool is_authenticated();

  bool anthenticate = false;
  int timeRef = 0;

  virtual  String responseHTML();
  String headHtml();
  virtual String createAdminPage();
  String incorrectPassword();
  virtual String formAdminPage() ;
  virtual void handleLogin();

private:
  Config& config;
  bool wantReboot;

  String formWifiPage() ;
  String info();

  
};
#endif