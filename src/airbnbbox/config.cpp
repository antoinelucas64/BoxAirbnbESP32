#include "config.h"

Config::Config() {
  config = new Preferences();
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

  return line("type=",type)
        +line(KEY_SSID,getSSID())
        + line(KEY_PASSWORD, getPassword()) 
        + line(KEY_WWW_USER, getWWWUser())
        + line(KEY_WWW_PASSWORD , getWWWPassword())
        + line("Version","1")
        + line("Date", __DATE__ );
}

void Config::factoryReset(){
  config->begin("config", false);
  config->putString(KEY_SSID, "Appartement Associes");
  config->putString(KEY_PASSWORD, "totototo");
  config->putString(KEY_WWW_USER, "admin");
  config->putString(KEY_WWW_PASSWORD, "toto");
  config->end();
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
  config->begin(config_file, true);
  ssid = config->getString(KEY_SSID, ssid);
  password = config->getString(KEY_PASSWORD, password);
  www_password = config->getString(KEY_WWW_PASSWORD, www_password);
  www_user = config->getString(KEY_WWW_USER, www_user);
  config->end();
}

// duplicate function to avoir 2 write to the file.
void Config::writeAllConfig(){
  config->begin(config_file, false);
  config->putString(KEY_SSID, ssid);
  config->putString(KEY_PASSWORD, password);
  config->putString(KEY_WWW_PASSWORD, www_password);
  config->putString(KEY_WWW_USER, www_user);
  config->end();
}

void Config::writeConfig(const String& password_p) {
  password = password_p;
  config->begin(config_file, false);
  config->putString(KEY_PASSWORD, password);
  config->end();
}

void Config::writeConfig(const String & ssid_p,const String& password_p) {
  password = password_p;
  ssid = ssid_p;
  config->begin(config_file, false);
  config->putString(KEY_SSID, ssid);
  config->putString(KEY_PASSWORD, password);
  config->end();
}


void Config::writeWWWConfig(const String & user, const String& password_p) {
  www_password = password_p;
  www_user = user;
  config->begin(config_file, false);
  config->putString(KEY_WWW_PASSWORD, www_password);
  config->putString(KEY_WWW_USER, www_user);
  config->end();
}

// duplicate function to avoid 2 write to the file.
void Config::writeAllConfig(const String & ssid_p, const String & password_p,const String & wwwUser,const String & wwwPassword){
  password = password_p;
  ssid = ssid_p;
  www_password =  wwwPassword;
  www_user = wwwUser;
  writeAllConfig();

}

