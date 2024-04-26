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
// SIM800H #define RELAY_ELEC 12
// SIM800L
#define RELAY_ELEC 25
#define POWER_ON  HIGH
#define POWER_OFF  LOW


class ConfigSim : public Config {

private:
  // mutex on phones vector as web & sim on different threads.
  aa::Mutex * lockArray;
  aa::Mutex * lockGpio;
  std::vector<String> * phones ;
  std::vector<String> * extraPhones ;
  int powerState = 0;

public:
  ConfigSim();
  ~ConfigSim(){};
  void init();
  void readSimConfig();
  void writePowerState(int powerState);
  void writePhones(std::vector<String> & phones);
  void writeExtraPhones(std::vector<String> & phones);
  virtual void writeAllConfig();
  static String arrayToConfig(const std::vector<String>  & array) ;
  static void configToArray(String configData, std::vector<String> & array) ;

  //void writeAllConfig(String & phones, String & extraPhones, int powerState);


  bool phoneIsAllowed(const char * phone, bool includesExtra = false) const;
  virtual String info() const;

  std::vector<String> getExtraPhones() const;
  std::vector<String> getPhones() const;

  void setPhones(const std::vector<String> & src);
  void setExtraPhones(const std::vector<String> & src);

  inline aa::Mutex * getMutex(){
    return lockGpio;
  }
  
  inline int getPowerState() const {
    return powerState;
  }

  inline void setPowerState(int power){
    powerState = power;
  }

protected:
  virtual String getType() const;

};



#endif