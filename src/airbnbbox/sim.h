#ifndef SIM_A_A
#define SIM_A_A

#include "configsim.h"
#include "Adafruit_FONA.h"

class Sim {
public:
  Sim(ConfigSim& config);
  ~Sim(){};

  void init();
  void loop();

  void reboot();
  static const int MAX_SMS_SIZE = 128 ;
private:
  Sim(const Sim &);
  ConfigSim& config;
  Adafruit_FONA * sim800l;
  void sendSMS(const char * telephone);
  void sendSMS(const char * telephone, const String & msg);
  /** split sms into n sub messages */
  void sendBigSMS(const char * telephone,const String & msg,const String & key);
};
#endif