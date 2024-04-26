#ifndef WEBSIM
#define WEBSIM

#include "configsim.h"
#include "web.h"
#include "sim.h"

class WebSim : public Web {

public:
  WebSim(ConfigSim& config, Sim & sim);
  virtual ~WebSim(){};

  virtual void reboot();
  void init();
private:
  ConfigSim& config;
  Sim & sim;
  String formWifiPage();
  String formPhone();

protected:
  virtual  String responseHTML();
  virtual String formAdminPage() ;
  virtual void handleLogin();

  virtual String createAdminPage();
};




#endif