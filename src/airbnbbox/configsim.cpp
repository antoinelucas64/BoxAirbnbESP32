#include "configsim.h"
#include <algorithm>

ConfigSim::ConfigSim() :
Config(),
 lockArray(new aa::Mutex("array")),
 lockGpio(new aa::Mutex("gpio",30)),
 phones(new std::vector<String>),
extraPhones (new std::vector<String>()),
powerState(0){

}

/*: 
  Config(),
  lockArray(),
  lockGpio(),
  phones(),
  extraPhones(),
  powerState(0){

}*/

void ConfigSim::init(){
  Config::init();
  readSimConfig() ;
}

String ConfigSim::getType() const{
  return "sim";
}


void ConfigSim::writePowerState(int powerState_p) {
  if(powerState == powerState_p) return;
  powerState = powerState_p;
  Serial.println("Write 0");
  config->begin(config_file, false);
  Serial.println("Write 1");
  config->putInt(KEY_POWER, powerState);
  Serial.println("Write 2");

  if (powerState) Serial.println("Write  file power 1");
  else Serial.println("Write  file power 0");
  config->end();
  LockMutex lock(lockGpio);
  digitalWrite(RELAY_ELEC, powerState_p);
}

String ConfigSim::arrayToConfig(const std::vector<String>  & array) {
 String retVal = "";
  for(int i = 0 ; i < array.size(); i++){
    retVal += array[i] + ";";
  }
  return retVal;
}

/** set array from values in configData - split by ";" */
void ConfigSim::configToArray(String configData, std::vector<String> & array) {
  array.clear();
  int length = configData.length();
  int index = 0;
  for (int i = 0 ; i < length; i++){
    if(configData[i] == ';' || i == length - 1){
      String tel;
      if(configData[i] == ';' ) tel =configData.substring(index,i);
      else tel =configData.substring(index,i+1);
      Serial.println("Phone "+ tel);
      array.push_back(tel);
      index=i+1;
    }
  }
}

void ConfigSim::writePhones(std::vector<String>  & phones_p) {
  LockMutex lock(lockArray);
  *phones = phones_p;

  String phone = arrayToConfig(phones_p);
  config->begin(config_file, false);
  config->putString(KEY_PHONE, phone);

  config->end();
}


void ConfigSim::writeExtraPhones(std::vector<String>  & extraPhones_p) {
  LockMutex lock(lockArray);
  *extraPhones = extraPhones_p;
  
  String phone = arrayToConfig(extraPhones_p);
  config->begin(config_file, false);
  config->putString(KEY_EXTRA_PHONE, phone);
  config->end();
}
void ConfigSim::writeAllConfig(){
  Config::writeAllConfig();
  LockMutex lock(lockArray);
  String phone = arrayToConfig(*phones);
  String exPhone = arrayToConfig(*extraPhones);

  config->begin(config_file, false);
  config->putString(KEY_PHONE, phone);
  config->putString(KEY_EXTRA_PHONE, exPhone);
  config->putInt(KEY_POWER, powerState);
  config->end();

}
/*
void ConfigSim::writeAllConfig(String & phones_p, String & extraPhones_p, int powerState_p){
  LockMutex lock(lockArray);
  configToArray(phones_p,*phones);
  configToArray(extraPhones_p,*extraPhones);
  powerState = powerState_p;
  config->begin(config_file, false);
  config->putString(KEY_PHONE, phones_p);
  config->putString(KEY_EXTRA_PHONE, extraPhones_p);
  config->putInt(KEY_POWER, powerState);
  config->end();

}
*/

void ConfigSim::readSimConfig() {
  LockMutex lock(lockArray);

  config->begin(config_file, true);
  phones->clear();
  extraPhones->clear();
  configToArray(config->getString(KEY_PHONE), *phones);
  configToArray(config->getString(KEY_EXTRA_PHONE), *extraPhones);

  powerState = config->getInt(KEY_POWER);
  config->end();
  Serial.println("Read all sim config");
}

/** extract last 8 digits of phone - this is to have 0612345678 == +33612345678 */
const char * extractSubPhone(const char * phone){
  int len = strlen(phone);
  if(len < 9 ) return phone;
  return phone + len - 9;
}

bool ConfigSim::phoneIsAllowed(const char * phone,bool includesExtra) const {
  if(phones->empty()) return true;


  Serial.print("Phone ");
  Serial.print(phone);
  Serial.print(" -> ");
  Serial.println(extractSubPhone(phone));
  for (int i = 0 ; i < phones->size(); i++){
   if(strstr(extractSubPhone(phone),extractSubPhone( (*phones)[i].c_str())) != NULL) return true;
  }
  if(includesExtra){
   for (int i = 0 ; i < extraPhones->size(); i++){
    if(strstr(extractSubPhone(phone),extractSubPhone( (*extraPhones)[i].c_str())) != NULL) return true;
    } 
  }
  return false;
}



std::vector<String> ConfigSim::getExtraPhones() const{
  LockMutex lock(lockArray);
  std::vector<String> copy = *extraPhones;
  return copy;
}

std::vector<String> ConfigSim::getPhones() const{
  LockMutex lock(lockArray);
  std::vector<String> copy = *phones;
  return copy;
}

void ConfigSim::setPhones(const std::vector<String> & src){
  LockMutex lock(lockArray);
  *phones = src;
}

void ConfigSim::setExtraPhones(const std::vector<String> & src){
  LockMutex lock(lockArray);
  *extraPhones = src;
}

String ConfigSim::info() const {
  return line(KEY_PHONE, arrayToConfig(*phones))
          + line(KEY_EXTRA_PHONE,arrayToConfig(*extraPhones))
          + line(KEY_POWER, String(powerState) )
          + Config::info();
}
