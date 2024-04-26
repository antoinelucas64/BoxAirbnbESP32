#ifndef CONFIG_H
#define CONFIG_H

#include <WString.h>
#ifdef ESP32
#define LED_BLUE  13
#include <Preferences.h>
#else
#define LED_BLUE  2
#include "Preferences.h"
#endif


// pins possible: 2 (4) 12 13
// SIM800H #define RELAY_DOOR 14
// SIM800L
#define RELAY_DOOR 19
#define DOOR_OPEN  LOW
#define DOOR_CLOSE  HIGH



// COM
#define KEY_PASSWORD "password"
#define KEY_SSID "ssid"
#define KEY_WWW_USER "www_user"
#define KEY_WWW_PASSWORD "www_password"
#define KEY_PHONE "phone"
#define KEY_EXTRA_PHONE "extra_phone"
#define KEY_POWER "state"

class Config{
public:
   

  Config();
  virtual ~Config(){};

  void init();

  void factoryReset();

  virtual String info() const;
  virtual void writeAllConfig();

  void writeConfig(const String & password);
  void writeConfig(const String & ssid, const String & password);
  void writeWWWConfig(const String & user,const String & password);
  void writeAllConfig(const String & ssid, const String & password,const String & wwwUser,const String & wwwPassword);
  
  static inline String line(const char * key, const String & value) {
    String out = "";
    return out + key + "=" + value + '\n';
  }

  inline const String & getSSID() const {
    return ssid;
  }
  inline const String & getPassword() const {
    return password;
  }
   
  inline const String & getWWWUser() const {
    return www_user;
  }  

  inline const String & getWWWPassword() const {
    return www_password;
  }  

  inline void setSSID(const String & ssid_p){
    ssid = ssid_p;
  }

  inline void setPassword(const String & password_p){
    password = password_p;
  }

  inline void setWWWUser(const String & user_p)  {
    www_user = user_p;
  }

  inline void setWWWPassword(const String & password_p)  {
    www_password = password_p;
  }



private:
  void readConfig();
  void blink();
  Config(const Config &);

  String ssid = "Appartement Associes";
  String password = "totototo";

  String www_user = "admin";
  String www_password = "toto";


protected:
  const char * config_file = "config";
  virtual String getType() const;
  Preferences  * config;
};
#endif
