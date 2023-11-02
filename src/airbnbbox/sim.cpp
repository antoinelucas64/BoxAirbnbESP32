#include "sim.h"
#include "StringUtils.h"
#include <WiFi.h>

#define MODEM_POWER_ON SIM800L_POWER
#define MODEM_RST 5
#define MODEM_PWRKEY 4

bool simOn = false;
bool firstStart = true;
HardwareSerial* sim800lSerial = &Serial1;
int simTimeRef = 0;

Sim::Sim(ConfigSim& config_p)
  : config(config_p),
    sim800l(Adafruit_FONA(SIM800L_PWRKEY)) {
}

void Sim::init() {
    Serial.println(F("Init Sim"));
  

#ifdef MODEM_RST
pinMode(MODEM_RST, OUTPUT);
digitalWrite(MODEM_RST, HIGH); // 5
#endif

pinMode(MODEM_PWRKEY, OUTPUT);   // 4
pinMode(MODEM_POWER_ON, OUTPUT); // 23
digitalWrite(MODEM_POWER_ON, HIGH);

digitalWrite(MODEM_PWRKEY, HIGH);
delay(100);
digitalWrite(MODEM_PWRKEY, LOW);
delay(1000);
digitalWrite(MODEM_PWRKEY, HIGH);

  simOn = true;

  simTimeRef = millis();
  delay(6000);

  // Make it slow so its easy to read!
  sim800lSerial->begin(115200, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  if (!sim800l.begin(*sim800lSerial)) {
    Serial.println(F("Couldn't find GSM SIM800L"));
    while (1);
  }
  Serial.println(F("GSM SIM800L is OK"));

  char imei[16] = { 0 };  // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = sim800l.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: ");
    Serial.println(imei);
  }

  // Set up the FONA to send a +CMTI notification
  // when an SMS is received
  sim800lSerial->print("AT+CNMI=2,1\r\n");

  Serial.println("GSM SIM800L Ready");
  if(firstStart) sendSMS(config.getPhone().c_str());
  firstStart = false;
}

void Sim::sendSMS(const char* telephone) {
  String msg = "Power is ";
  if (digitalRead(RELAY_ELEC) == POWER_ON) msg += "ON ";
  else msg += "OFF ";
  msg += "password is " + config.getPassword();
  delay(100);
  if (!sim800l.sendSMS(telephone, msg.c_str())) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }
  delay(100);
  sim800l.sendCheckReply("AT+CMGDA=\"DEL ALL\"", F("> "));
  delay(100);
  sim800l.sendCheckReply("AT+CMGDA=6", F("> "));
}


void Sim::loop() {
  char buffer[250];
  if(!simOn) {
    if( millis() - simTimeRef > 1000 ){
         init();
    } else return;
  }

  if (sim800l.available()) {
    int slot = 0;  // this will be the slot number of the SMS
    int charCount = 0;

    // Read the notification into fonaInBuffer
    for (int i = 0 ; i < sizeof(buffer)-1 ; i++){
      buffer[i] = sim800l.read();
      if(buffer[i] == '\n'){
        buffer[i+1] = 0;
        break;
      }
    }
   

    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(buffer, "+CMTI: \"SM\",%d", &slot)) {
      Serial.print("slot: ");
      Serial.println(slot);
      digitalWrite(LED_BLUE, HIGH);

      String smsString = "";
      char phone[32];
      // Retrieve SMS sender address/phone number.
      if (!sim800l.getSMSSender(slot, phone, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM = "));
      Serial.println(phone);
      if (!aa::contains(phone, config.getPhone().c_str())) {
        sim800l.deleteSMS(slot);
        Serial.println("SMS ignored");
        return;
      }
      // Retrieve SMS value.
      uint16_t smslen;
      // Pass in buffer and max len!
      if (sim800l.readSMS(slot, buffer, 250, &smslen)) {
        smsString = String(buffer);
        Serial.println(smsString);
      } 
      if (aa::contains(smsString.c_str(), "REBOOT")) {
        ESP.restart();
      }
      else if (aa::contains(smsString.c_str(), "PASSWORD")) {
        String password = aa::secondWord(smsString);
        if (password.length() > 7) {
          Serial.println("Change password to " + password);
          if (WiFi.softAP(config.getSSID().c_str(), password.c_str())) {
            config.writeConfig(password);
          }

        } else {
          Serial.println("incorrect password ");
        }
        sendSMS(phone);

      } else if (aa::contains(smsString.c_str(), "ON")) {
        Serial.println("Relay is activated.");
        if (config.getPowerState() != POWER_ON) {
          config.writePowerState(POWER_ON);
        }
        // Send SMS for status
        sendSMS(phone);
      } else if (aa::contains(smsString.c_str(), "OFF")) {
        Serial.println("Relay is deactivated.");
        if (config.getPowerState() != POWER_OFF) {
          config.writePowerState(POWER_OFF);
        }
        // Send SMS for status
        sendSMS(phone);

      } else {
        Serial.println("message ignore (command not found)");
      }

      if (sim800l.deleteSMS(slot)) {
        Serial.println(F("OK!"));
      } 
      digitalWrite(LED_BLUE, LOW);
    }
  } 

  if( simOn ){
    if( millis() - simTimeRef > 1200000 ){          
        Serial.println("shut down sim ");
        simOn = false;
        digitalWrite(SIM800L_POWER, LOW);
        simTimeRef = millis();
    }
  } 
}
