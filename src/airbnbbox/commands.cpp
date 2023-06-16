#include "config.h"


void openDoor(){
    digitalWrite(LED_BLUE, HIGH);
    Serial.println("open\nHIGH");
    digitalWrite(RELAY_DOOR,DOOR_OPEN);
    delay(10000);
    digitalWrite(RELAY_DOOR,DOOR_CLOSE);
    digitalWrite(LED_BLUE, LOW);

}
