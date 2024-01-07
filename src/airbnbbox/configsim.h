#ifndef CONFIG_SIM
#define CONFIG_SIM

#include "config.h"
#include <vector>
#include "mutex.h"
#define SIM800L_RX     27
#define SIM800L_TX     26
#define MODEM_PWRKEY 4
#define MODEM_RST    5
#define SIM800L_POWER  23
#define RELAY_ELEC 12
#define POWER_ON  LOW
#define POWER_OFF  HIGH


class ConfigSim : public Config {

private:
  // mutex on phones vector as web & sim on different threads.
  aa::Mutex lockArray;
  aa::Mutex lockGpio;
  std::vector<String> phones ;
  std::vector<String> extraPhones ;
  bool powerState = 0;

public:
  ConfigSim();
  ~ConfigSim(){};
  void init();
  void readSimConfig();
  void writePowerState(bool powerState);
  void writePhones(std::vector<String> & phones);
  void writeExtraPhones(std::vector<String> & phones);

  void writeAllConfig(String & phones, String & extraPhones, bool powerState);


  bool phoneIsAllowed(const char * phone, bool includesExtra = false);
  virtual String info() const;

  std::vector<String> getExtraPhones() ;
  std::vector<String> getPhones();

  inline aa::Mutex * getMutex(){
    return &lockGpio;
  }
  
  inline bool getPowerState() const {
    return powerState;
  }

protected:
  virtual String getType() const;

};



#endif