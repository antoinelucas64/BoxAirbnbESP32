#include "config.h"

Config::Config() {
 }

void Config::init(){
  readConfig();
}

String Config::getType() const {
  return "basic";
}

String Config::info() const {
#ifdef ESP32
  String type = getType()+ "-ESP32" ;
#else
  String type = getType()+ "-ESP8266" ;
#endif

  return "type=\t"+type+"\nSSID=\t"+getSSID() + "\nPassword=\t" + getPassword() + "\nWeb user=\t" + getWWWUser() + "\nWeb password=\t" + getWWWPassword() + "\nVersion=\t 1\nDate=\t" + __DATE__ + "\n";
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

// duplicate function to avoir 2 write to the file.
void Config::writeAllConfig(const String & ssid_p, const String & password_p,const String & wwwUser,const String & wwwPassword){
  password = password_p;
  ssid = ssid_p;
  www_password =  wwwPassword;
  www_user = wwwUser;
  config.begin("config", false);
  config.putString("ssid", ssid);
  config.putString("password", password);
  config.putString("www_password", www_password);
  config.putString("www_user", www_user);
  config.end();

}

