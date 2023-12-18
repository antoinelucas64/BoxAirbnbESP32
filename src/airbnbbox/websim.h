#ifndef WEBSIM
#define WEBSIM

#include "configsim.h"
#include "web.h"

class WebSim : public Web {

public:
  WebSim(ConfigSim& config);
  virtual ~WebSim(){};

  void init();
private:
  ConfigSim& config;

  String formWifiPage();
  String formPhone();

protected:
  virtual  String responseHTML();

  virtual String createAdminPage();
};




#endif