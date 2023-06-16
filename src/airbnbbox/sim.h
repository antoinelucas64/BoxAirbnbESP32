#ifndef SIM_A_A
#define SIM_A_A

#include "config.h"
#include "Adafruit_FONA.h"

class Sim {
public:
  Sim(Config& config);
  ~Sim(){};

  void init();
  void loop();
private:
  Config& config;
  Adafruit_FONA sim800l;
  void sendSMS(const char * telephone);
};
#endif