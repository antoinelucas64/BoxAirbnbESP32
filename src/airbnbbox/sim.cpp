#include "sim.h"
#include "StringUtils.h"
#include <WiFi.h>
#include "commands.h"
#include <math.h>

bool simOn = false;
bool firstStart = true;
HardwareSerial* sim800lSerial = &Serial1;
int simTimeRef = 0;

Sim::Sim(ConfigSim& config_p)
  : config(config_p),
    sim800l(new Adafruit_FONA(MODEM_PWRKEY)) {
}

void Sim::init() {

  Serial.println(F("Init Sim"));
  {
    LockMutex lock(config.getMutex());

#ifdef MODEM_RST
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);  // 5
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);   // 4
    pinMode(SIM800L_POWER, OUTPUT);  // 23
    digitalWrite(SIM800L_POWER, HIGH);

    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    //delay(1000);
    //digitalWrite(MODEM_PWRKEY, HIGH);
  }


  simOn = true;

  simTimeRef = millis();
  delay(6000);

  // avoid digitalWrite with several threads at same time
  {
    LockMutex lock(config.getMutex());
    // Make it slow so its easy to read!
    sim800lSerial->begin(115200, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
    //sim800lSerial->begin(600, SERIAL_8N1, SIM800L_TX, SIM800L_RX);


    if (!sim800l->begin(*sim800lSerial)) {
      Serial.println(F("Couldn't find GSM SIM800L"));
      simOn = false;
      return;
    }
  }
  sim800l->readFromSim(5000);  // CALL READY
  sim800l->readFromSim(5000);  // SMS READY
  sim800l->readFromSim(500);

  Serial.println(F("GSM SIM800L is OK"));

  // reset sim
  //sim800l.sendCheckReply("ATZ", F("> "));

  // CMGF: 1 mode text / 0 mode PDU
  sim800l->sendCheckReply(F("AT+CMGF=1"), F("> "), 2000);  // OK

  // list messages
  //sim800l.sendCheckReply(F("AT+CMGL?"), F("> "));         // error
  // sim800l.sendCheckReply(F("AT+CMGL=?"), F("> "));        // error
  sim800l->sendCheckReply(F("AT+CMGL=\"ALL\""), F("> "), 20000);  // OK
  //sim800l.sendCheckReply(F("AT+CMGL=4"), F("> "),4000);        // error
  // storage to modem
  //sim800l.sendCheckReply("AT+CPMS?", F("> "));                      // +CPMS: "ME",0,50,"SM",9,50,"MT",9,100
  //sim800l.sendCheckReply("AT+CPMS=?", F("> "));                     // +CPMS: ("SM","ME","SM_P","ME_P","MT"),("SM","ME","SM_P","ME_P","MT"),("SM","ME","SM_P","ME_P","MT")
  sim800l->sendCheckReply("AT+CPMS=\"ME\",\"ME\",\"ME\"", F("> "));  // error
  //sim800l.sendCheckReply("AT+CSCS=?", F("> ")); // +CSCS: ("IRA","GSM","UCS2","HEX","PCCP","PCDN","8859-1")
  sim800l->sendCheckReply("AT+CSCS=\"GSM\"", F("> "));  // OK
  //sim800l.sendCheckReply("AT+CSTA=?", F("> "));//+CSTA: (129,145,161,177)

  // sim800l.sendCheckReply("AT+CLIP=?", F("> "),3000);
  // sim800l.sendCheckReply("AT+CLIP?", F("> "),3000);
  sim800l->sendCheckReply("AT+CLIP=1", F("> "), 3000);

  // CFUN=0 minmum function CFUN=1 normal CFUN=4 mode avion
  //sim800l.sendCheckReply("AT+CFUN=?", F("> "),3000);
  //sim800l.sendCheckReply("AT+CFUN?", F("> "),3000);
  //sim800l.sendCheckReply("AT+CFUN=1", F("> "),3000);

  // phone / fax / data
  //  sim800l.sendCheckReply("AT+CSNS?",F("> ")); //
  //  sim800l.sendCheckReply("AT+CSNS=?",F("> ")); //
  //  sim800l.sendCheckReply("AT+CSNS=4",F("> ")); //


  // CNMI command supported
  //sim800l.sendCheckReply("AT+CNMI=?", ">",1500);

  // not to store sms
  //sim800l.sendCheckReply("AT+CNMI?", F("> "),3000);
  //sim800l.sendCheckReply("AT+CNMI=3,3,2,1,0", F("> "),2000);
  //  sim800l.sendCheckReply("AT+CNMI=2,2",F("> "));
  // list of  storage type
  //sim800l.sendCheckReply("AT+CMGDA=?", F("> "),1000);

  //list of supported index to delete
  //sim800l.sendCheckReply("AT+CMGD=?", F("> "),1000);

  char imei[16] = { 0 };  // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = sim800l->getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: ");
    Serial.println(imei);
  }

  // Set up the FONA to send a +CMTI notification
  // when an SMS is received
  sim800lSerial->print("AT+CNMI=2,1\r\n");

  Serial.println("GSM SIM800L Ready");
  if (firstStart && !config.getPhones().empty()) sendSMS(config.getPhones()[0].c_str());

  firstStart = false;
}

void Sim::sendSMS(const char* telephone) {
  String msg = "Power is ";
  if (config.getPowerState() == POWER_ON) msg += "ON ";
  else msg += "OFF ";
  msg += "password is " + config.getPassword() + " SSID " + config.getSSID();
  sendSMS(telephone, msg);
}

// message will be
//
// 03FirstMessage\nkey
// 13SecondMessage\nkey
// 23ThirdMessage\nkey
//

void Sim::sendBigSMS(const char* telephone, const String & msg, const String & key) {

  if (msg.length() + key.length() + 1 <= MAX_SMS_SIZE) {
    sendSMS(telephone, msg +'\n' + key);
    return;
  }
  int index = 0;
  int sizeSubMsg = MAX_SMS_SIZE - key.length() - 2;
  int count = 0;
  int total = ceil( (0.+msg.length() ) / sizeSubMsg);
  while(index < msg.length()){
    String subMsg = String(count++) +String(total)+ msg.substring(index,index+ sizeSubMsg) + '\n' + key;
    index+= sizeSubMsg;
    sendSMS(telephone, subMsg);
  }
}

void Sim::sendSMS(const char* telephone,const String & msg) {
  delay(100);
  Serial.print("Try send message ");
  Serial.print(msg);
  Serial.print(" to phone ");
  Serial.println(telephone);
  if (!sim800l->sendSMS(telephone, msg.c_str())) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }
  delay(100);
  Serial.print("nb sms ");
  Serial.println(sim800l->getNumSMS());
  sim800l->sendCheckReply("AT+CMGDA=6", F("> "), 3500);
  sim800l->sendCheckReply("AT+CMGD=,4", F("> "), 1500);
  delay(100);

  Serial.println(F("Sent DEL ALL"));
  sim800l->sendCheckReply("AT+CMGDA=\"DEL ALL\"", F("> "), 25000);

  Serial.println("FIN");
  int MessageQtt = 0;
  Serial.print(F("["));
  do {
    sim800l->flush();
    MessageQtt++;
  } while (sim800l->available() > 0);
  Serial.print(MessageQtt);
  sim800l->flush();
  Serial.println("] done");
}


void Sim::reboot() {
  // resert modem
  sim800l->sendCheckReply("ATZ", F("> "));
  sim800l->flush();
  ESP.restart();
}

void Sim::loop() {
  char buffer[255];
  if (!simOn) {
    if (millis() - simTimeRef > 1000) {
      init();
    } else {
      Serial.println(F("Sim off"));
      return;
    }
  }

  if (sim800l->available()) {
    Serial.println("hey something here. try read");
    int slot = 0;  // this will be the slot number of the SMS
    int charCount = 0;

    // Read the notification into fonaInBuffer
    for (int i = 0; i < sizeof(buffer) - 1; i++) {
      buffer[i] = sim800l->read();
      if (buffer[i] == '\n') {
        buffer[i + 1] = 0;
        break;
      }
    }


    Serial.print("buffer> ");
    Serial.println(buffer);

    if (strncmp(buffer, "RING", 4) == 0) {
      Serial.println("RING -> possible action here");
      return;
    }

    // +CLIP: "0612345678",129,"",0,"",0
    if (strncmp(buffer + 1, "CLIP", 4) == 0) {
      Serial.println("Appel avec numero -> possible action here");
      char tel[256];
      bool readClip = sscanf(buffer, "+CLIP: \"%s\",", &tel) == 1;
      // tel is something like 0123456789",129,"",0,"",0 -> ",0,"",0
      for (int i = 1; i < 255; i++) {
        if (tel[i] == '"') tel[i] = '\0';
      }
      if (readClip) {
        Serial.print("Read phone ");
        Serial.println(tel);
        if (config.phoneIsAllowed(tel, true)) {
          Serial.println("Granted");
        }
      }

      return;
    }

    if (strncmp(buffer + 1, "CMTI", 4) != 0) {
      Serial.println("ignore");
      return;
    }


    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    // read something like [+CMTI: "ME",53]
    bool readCMTI = sscanf(buffer, "+CMTI: \"SM\",%d", &slot) == 1;
    if (!readCMTI) readCMTI = sscanf(buffer, "+CMTI: \"ME\",%d", &slot) == 1;
    else if (!readCMTI) readCMTI = sscanf(buffer, "+CMTI: \"MT\",%d", &slot) == 1;
    else if (!readCMTI) readCMTI = sscanf(buffer, "+CMTI: \"ME_P\",%d", &slot) == 1;
    else if (!readCMTI) readCMTI = sscanf(buffer, "+CMTI: \"SM_P\",%d", &slot) == 1;
    if (readCMTI) {
      Serial.print("slot: ");
      Serial.println(slot);
      digitalWrite(LED_BLUE, HIGH);

      String smsString = "";
      char phone[32];
      // Retrieve SMS sender address/phone number.
      if (!sim800l->getSMSSender(slot, phone, 31)) {
        Serial.println("Didn't find SMS message in slot!");
        delay(600);
        sim800l->getSMSSender(slot, phone, 31);
      }
      Serial.print(F("FROM = "));
      Serial.println(phone);
      if (!config.phoneIsAllowed(phone)) {
        sim800l->deleteSMS(slot);
        Serial.println("SMS ignored");
        digitalWrite(LED_BLUE, LOW);
        return;
      }
      // Retrieve SMS value.
      uint16_t smslen;
      // Pass in buffer and max len!
      if (sim800l->readSMS(slot, buffer, 255, &smslen)) {
        smsString = buffer;
        /*if(smslen == 250){
          sim800l->readSMS(slot, buffer, 250, &smslen);
          smsString += buffer;
        }*/
        Serial.println("SMS READ ->");
        Serial.println(smsString);
        Serial.print("<- SMS READ size ");
        Serial.println(smslen);
      }
      if (aa::contains(smsString.c_str(), "REBOOT")) {
        // reset SIM
        reboot();
      } else if (aa::contains(smsString.c_str(), "UPDATE")) {
        int indexLigne =-1;
        int indexEqual=-1;
        for (int i = 0; i < smsString.length(); i++){
          if(smsString[i] == '='){
            indexEqual = i;
          } else if (smsString[i] == '\n'){
            if(indexEqual <= indexLigne) {
              indexLigne = i;
              continue;
            }
            String keyS = smsString.substring(indexLigne+1,indexEqual);

            const char * key = keyS.c_str();
            Serial.print("key:");
            Serial.println(keyS);
            String value = smsString.substring(indexEqual+1, i);
            Serial.print("value:");
            Serial.println(value);
            indexLigne = i;

            if(aa::sameWords(KEY_PASSWORD,key)){
              Serial.println("update password ");
              
              if (value.length()>7 && WiFi.softAP(config.getSSID().c_str(), value.c_str())) {
                config.setPassword(value);
              } 
            } else if (aa::sameWords(KEY_WWW_USER, key)){
              config.setWWWUser(value);
            } else if (aa::sameWords(KEY_WWW_PASSWORD, key)){
              config.setWWWPassword(value);
            } else if (aa::sameWords(KEY_SSID, key)){
              config.setSSID(value);
            } else if (aa::sameWords(KEY_PHONE, key)){
              std::vector<String> phones;
              ConfigSim::configToArray(value, phones);
              config.setPhones(phones);
            } else if (aa::sameWords(KEY_EXTRA_PHONE, key)){
              std::vector<String> phones;
              ConfigSim::configToArray(value, phones);
              config.setExtraPhones(phones);
            }else if (aa::sameWords(KEY_POWER, key)){
              config.setPowerState(aa::sameWords(value.c_str(), "1") ? 1 : 0);
            }
          }
        }
        config.writeAllConfig();
        

     } else if (aa::contains(smsString.c_str(), "OPEN")) {
        openDoor();
      } else if (aa::contains(smsString.c_str(), "INFO")) {
        sendBigSMS(phone, config.info(),aa::secondWord(smsString));
      } else if (aa::contains(smsString.c_str(), "PASSWORD")) {
        String password = aa::secondWord(smsString);
        if (password.length() > 7 && WiFi.softAP(config.getSSID().c_str(), password.c_str())) {
          Serial.println("Change password to " + password);
          config.writeConfig(password);

        } else {
          Serial.println("incorrect password ");
        }
        sendSMS(phone);

      } else if (aa::contains(smsString.c_str(), "ON")) {
        Serial.println("Relay is activated.");
        if (config.getPowerState() != POWER_ON) {
          Serial.println("Write  file power - on");
          config.writePowerState(POWER_ON);
        }
        Serial.println("Relay is activated - ok.");

        // Send SMS for status
        sendSMS(phone);
      } else if (aa::contains(smsString.c_str(), "OFF")) {
        Serial.println("Relay is deactivated.");
        if (config.getPowerState() != POWER_OFF) {
          config.writePowerState(POWER_OFF);
        }
        Serial.println("Relay is deactivated. - ok");
        // Send SMS for status
        sendSMS(phone);

      } else {
        String msg = "Commands are ON, OFF, PASSWORD [new password], OPEN, REBOOT, INFO";
        sendSMS(phone, msg);
      }

      if (sim800l->deleteSMS(slot)) {
        Serial.println(F("OK!"));
      }
      digitalWrite(LED_BLUE, LOW);
    }
  }

  if (simOn) {
    // 6 000 000 is 100 minutes
    if (millis() - simTimeRef > 6000000) {
      //if (millis() - simTimeRef > 600000) {
      Serial.println("shut down sim ");
      simOn = false;
      digitalWrite(SIM800L_POWER, LOW);
      simTimeRef = millis();
    }
  }
}
