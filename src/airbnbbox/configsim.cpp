#include "configsim.h"

#define KEY_PHONE "phone"
#define KEY_EXTRA_PHONE "extra_phone"
#define KEY_POWER "state"

ConfigSim::ConfigSim(){

}

void ConfigSim::init(){
  Config::init();
  readSimConfig() ;
}

String ConfigSim::getType() const{
  return "sim";
}


void ConfigSim::writePowerState(bool powerState_p) {
  powerState = powerState_p;
  config.begin("config", false);
  config.putBool(KEY_POWER, powerState);

  if (powerState) Serial.println("Write  file power 1");
  else Serial.println("Write  file power 0");
  config.end();
  digitalWrite(RELAY_ELEC, powerState_p);
}

String arrayToConfig(const std::vector<String>  & array){
 String retVal = "";
  for(int i = 0 ; i < array.size(); i++){
    retVal += array[i] + ";";
  }
  return retVal;
}

/** set array from values in configData - split by ";" */
void configToArray(String configData, std::vector<String> & array){
  array.clear();
  int length = configData.length();
  int index = 0;
  for (int i = 0 ; i < length; i++){
    if(configData[i] == ';'){
      String tel=configData.substring(index,i);
      Serial.println("Phone "+ tel);
      array.push_back(tel);
      index=i+1;
    }
  }
}

void ConfigSim::writePhones(std::vector<String>  & phones_p) {
  pthread_mutex_lock(&mutex);
  phones = phones_p;
  pthread_mutex_unlock(&mutex);

  String phone = arrayToConfig(phones_p);
  config.begin("config", false);
  config.putString(KEY_PHONE, phone);

  config.end();
}


void ConfigSim::writeExtraPhones(std::vector<String>  & extraPhones_p) {
  pthread_mutex_lock(&mutex);
  extraPhones = extraPhones_p;
  pthread_mutex_unlock(&mutex);

  String phone = arrayToConfig(extraPhones_p);
  config.begin("config", false);
  config.putString(KEY_EXTRA_PHONE, phone);
  config.end();
}

void ConfigSim::writeAllConfig(String & phones_p, String & extraPhones_p, bool powerState_p){
  pthread_mutex_lock(&mutex);
  configToArray(phones_p,phones);
  configToArray(extraPhones_p,extraPhones);
  pthread_mutex_unlock(&mutex);
  powerState = powerState_p;
  config.begin("config", false);
  config.putString(KEY_PHONE, phones_p);
  config.putString(KEY_EXTRA_PHONE, extraPhones_p);
  config.putBool(KEY_POWER, powerState);
  config.end();

}


void ConfigSim::readSimConfig() {
  pthread_mutex_lock(&mutex);

  config.begin("config", true);
  phones.clear();
  extraPhones.clear();
  configToArray(config.getString(KEY_PHONE), phones);
  configToArray(config.getString("extraPhones"), extraPhones);

  powerState = config.getBool(KEY_POWER);
  config.end();
  pthread_mutex_unlock(&mutex);

}

/** extract last 8 digits of phone - this is to have 0612345678 == +33612345678 */
const char * extractSubPhone(const char * phone){
  int len = strlen(phone);
  if(len < 9 ) return phone;
  return phone + len - 9;
}

bool ConfigSim::phoneIsAllowed(const char * phone,bool includesExtra){
  if(phones.empty()) return true;


  Serial.print("Phone ");
  Serial.print(phone);
  Serial.print(" -> ");
  Serial.println(extractSubPhone(phone));
  for (int i = 0 ; i < phones.size(); i++){
   if(strstr(extractSubPhone(phone),extractSubPhone( phones[i].c_str())) != NULL) return true;
  }
  if(includesExtra){
   for (int i = 0 ; i < extraPhones.size(); i++){
    if(strstr(extractSubPhone(phone),extractSubPhone( extraPhones[i].c_str())) != NULL) return true;
    } 
  }
  return false;
}

String ConfigSim::info() const {
  String infoConf = Config::info();
  return  "phones=\t"+arrayToConfig(phones)+"\nextraPhones=\t"+arrayToConfig(extraPhones) + "\npowerState=\t" + powerState + "\n" + infoConf;
}
