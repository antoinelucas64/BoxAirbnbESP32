#ifndef CONFIG_SIM
#define CONFIG_SIM

#include "config.h"
#define SIM800L_RX     27
#define SIM800L_TX     26
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23
#define RELAY_ELEC 12
#define POWER_ON  LOW
#define POWER_OFF  HIGH


class ConfigSim : public Config {

public:
  ConfigSim();
  ~ConfigSim(){};
  void init();
  void readSimConfig();
  void writePowerState(bool powerState);
  void writePhone(String phone);

  inline String getPhone() {
    return phone;
  }

  inline bool getPowerState() const {
    return powerState;
  }
private:

  String phone = "+3312345678";
  bool powerState = 0;
};



#endif