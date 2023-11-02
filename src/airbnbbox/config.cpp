#include "config.h"

Config::Config() {
 }

void Config::init(){
  readConfig();
}


String Config::info(){
  return "type=\tESP8266-1\nSSID=\t"+getSSID() + "\nPassword=\t" + getPassword() + "\nWeb user=\t" + getWWWUser() + "\nWeb password=\t" + getWWWPassword() + "Version=\t 1\nDate=\t" + __DATE__ + "\n";
}

void Config::factoryReset(){
  config.begin("config", false);
  config.putString("ssid", "Appartement Associes");
  config.putString("password", "totototo");
  config.putString("www_user", "admin");
  config.putString("www_password", "toto");
  config.end();
  blink();

}

void Config::blink() {
  bool ledState = true;
  while(true){
    digitalWrite(LED_BLUE,ledState);
    ledState = !ledState;
    delay(500);
  }
}

void Config::readConfig() {
  config.begin("config", true);
  ssid = config.getString("ssid", ssid);
  password = config.getString("password", password);
  www_password = config.getString("www_password", www_password);
  www_user = config.getString("www_user", www_user);
  config.end();
}

void Config::writeConfig(const String& password_p) {
  password = password_p;
  config.begin("config", false);
  config.putString("password", password);
  config.end();
}

void Config::writeConfig(const String & ssid_p,const String& password_p) {
  password = password_p;
  ssid = ssid_p;
  config.begin("config", false);
  config.putString("ssid", ssid);
  config.putString("password", password);
  config.end();
}


void Config::writeWWWConfig(const String & user, const String& password_p) {
  www_password = password_p;
  www_user = user;
  config.begin("config", false);
  config.putString("www_password", www_password);
  config.putString("www_user", www_user);
  config.end();
}

