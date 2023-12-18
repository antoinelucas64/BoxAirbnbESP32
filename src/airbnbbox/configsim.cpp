#include "configsim.h"


ConfigSim::ConfigSim(){

}

void ConfigSim::init(){
  Config::init();
  readSimConfig() ;
}

String ConfigSim::getType(){
  return "sim";
}


void ConfigSim::writePowerState(bool powerState_p) {
  powerState = powerState_p;
  config.begin("config", false);
  config.putBool("state", powerState);

  if (powerState) Serial.println("Write  file power 1");
  else Serial.println("Write  file power 0");
  config.end();
  digitalWrite(RELAY_ELEC, powerState_p);
}


void ConfigSim::writePhone(String phone_p) {
  phone = phone_p;
  config.begin("config", false);
  config.putString("phone", phone);

  config.end();
}


void ConfigSim::readSimConfig() {
  config.begin("config", true);
  phone = config.getString("phone",phone);
  powerState = config.getBool("state");
  config.end();
}
