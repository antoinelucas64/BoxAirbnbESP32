#ifndef WEB_A_A
#define WEB_A_A

#include "config.h"
#include <WebServer.h>

class Web {
public:
  Web(Config& config);
  ~Web(){};

  void init();
  void handleClient();
private:
  Config& config;
  WebServer server;

  String createAdminPage();
  String formWifiPage() ;
};
#endif