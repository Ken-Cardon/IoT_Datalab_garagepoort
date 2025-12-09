#include <Arduino.h>

int led_pin = 0;
int druk_pin = 4;

boolean druk_status = false;

void setup() {
  

 pinMode(led_pin, OUTPUT);
 pinMode(druk_pin, INPUT);
 digitalWrite(led_pin, LOW); // LED uit bij start
 
}

void loop() {

  druk_status = digitalRead(druk_pin);
  if(druk_status) {
    digitalWrite(led_pin, LOW);
    digitalWrite(led_pin, HIGH);
    delay(10000);
  }
  else{
    digitalWrite(led_pin, LOW);
  }

}