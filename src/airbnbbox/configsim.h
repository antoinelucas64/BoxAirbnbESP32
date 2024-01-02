#ifndef CONFIG_SIM
#define CONFIG_SIM

#include "config.h"
#include <vector>
#include <pthread.h>
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
  void writePhones(std::vector<String> & phones);
  void writeExtraPhones(std::vector<String> & phones);

  void writeAllConfig(String & phones, String & extraPhones, bool powerState);


  bool phoneIsAllowed(const char * phone, bool includesExtra = false);
  virtual String info() const;

  inline std::vector<String> getExtraPhones() {
    pthread_mutex_lock(&mutex);
    std::vector<String> copy = phones;
    pthread_mutex_unlock(&mutex);
    return copy;
  }
  inline std::vector<String> getPhones() {
    pthread_mutex_lock(&mutex);
    std::vector<String> copy = phones;
    pthread_mutex_unlock(&mutex);
    return copy;
  }

  inline bool getPowerState() const {
    return powerState;
  }
private:
  // mutex on phones vector as web & sim on different threads.
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  std::vector<String> phones ;
  std::vector<String> extraPhones ;
  bool powerState = 0;

protected:
  virtual String getType() const;

};



#endif