#include <Arduino.h>

const int ledPin = 0;
const int buttonPin = 4;

enum State { STOPPED, FORWARD, REVERSE };
State state = STOPPED;

bool lastBtn = false;

unsigned long startTime = 0;
unsigned long elapsed = 0;         // tijd dat beweging liep vóór stop
unsigned long targetDuration = 0;  // hoe lang deze beweging moet duren

const unsigned long MAX_DURATION = 10000; // 10 seconden

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT); // jouw externe pulldown
  digitalWrite(ledPin, LOW);
}

void loop() {
  bool currBtn = digitalRead(buttonPin);
  bool pressed = (currBtn && !lastBtn);
  lastBtn = currBtn;

  unsigned long now = millis();

  switch(state) {

    // ------------------------------------------------------
    // STOPPED
    // ------------------------------------------------------
    case STOPPED:
      digitalWrite(ledPin, LOW);

      if (pressed) {

        if (elapsed == 0) {
          // Start een NIEUWE beweging → FORWARD max 10 sec
          state = FORWARD;
          targetDuration = MAX_DURATION;
        } 
        else {
          // Start een REVERSE beweging met exact de vorige tijd
          state = REVERSE;
          targetDuration = elapsed;
        }

        startTime = now;
      }
      break;


    // ------------------------------------------------------
    // FORWARD
    // ------------------------------------------------------
    case FORWARD:
      digitalWrite(ledPin, HIGH);

      if (pressed) {
        // STOP
        unsigned long ran = now - startTime;
        if (ran > MAX_DURATION) ran = MAX_DURATION;
        elapsed = ran;       // bewaar gelopen tijd
        state = STOPPED;
      }
      else if (now - startTime >= targetDuration) {
        // AUTOMATISCHE STOP
        elapsed = 0;         // volledig traject → volgende keer weer FORWARD 10s
        state = STOPPED;
      }
      break;


    // ------------------------------------------------------
    // REVERSE
    // ------------------------------------------------------
    case REVERSE:
      digitalWrite(ledPin, HIGH);

      if (pressed) {
        // STOP
        unsigned long ran = now - startTime;
        if (ran > targetDuration) ran = targetDuration;
        elapsed = ran;       // bewaar tijd die reverse liep
        state = STOPPED;
      }
      else if (now - startTime >= targetDuration) {
        // AUTOMATISCHE STOP
        elapsed = 0;         // reverse volledig → volgende druk = FORWARD max 10s
        state = STOPPED;
      }
      break;
  }
}
