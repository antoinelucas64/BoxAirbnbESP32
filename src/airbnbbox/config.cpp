#include "config.h"

Config::Config() {
 }

void Config::init(){
  readConfig();
  readPowerState();
}

void Config::readConfig() {
  config.begin("config", true);
  ssid = config.getString("ssid", ssid);
  password = config.getString("password", password);
  www_username = config.getString("www_username", www_username);
  www_password = config.getString("www_password", www_password);

  config.end();
}

void Config::writeConfig(const String& password_p) {
  password = password_p;
  config.begin("config", false);
  config.putString("ssid", ssid);
  config.putString("password", password);
  config.putString("www_username", www_username);
  config.putString("www_password", www_password);
  config.end();
}

void Config::readPowerState() {
  powerperf.begin("power", true);
  powerState = powerperf.getBool("state");
  powerperf.end();
  if (powerState ) Serial.println("Read file power 1");
  else Serial.println("Read file power 0");
}

void Config::writePowerState(bool powerState_p) {
  powerState = powerState_p;
  powerperf.begin("power", false);
  powerperf.putBool("state", powerState);

  if (powerState) Serial.println("Write  file power 1");
  else Serial.println("Write  file power 0");
  powerperf.end();
}
