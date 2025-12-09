#include <Arduino.h>

const int ledPin = 0;
const int buttonPin = 4;

bool lastBtn = false;

// States
enum State { STOPPED, FORWARD, REVERSE };
State state = STOPPED;

unsigned long startTime = 0;
unsigned long elapsed = 0;   // tijd dat motor liep vóór stoppen

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT); // externe pulldown
  digitalWrite(ledPin, LOW); // LED uit bij start
}

void loop() {
  bool currBtn = digitalRead(buttonPin);
  bool pressed = (currBtn && !lastBtn);
  lastBtn = currBtn;

  unsigned long now = millis();

  switch(state) {

    case STOPPED:
      digitalWrite(ledPin, LOW);

      if (pressed) {
        if (elapsed == 0) {
          // eerste start → vooruit
          state = FORWARD;
          startTime = now;
        } else {
          // we hebben eerder gestopt → reverse exact elapsed
          state = REVERSE;
          startTime = now;
        }
      }
      break;


    case FORWARD:
      digitalWrite(ledPin, HIGH);

      if (pressed) {
        // stop
        elapsed = now - startTime;
        state = STOPPED;
      }
      break;


    case REVERSE:
      digitalWrite(ledPin, HIGH);

      if (pressed) {
        // stop opnieuw
        elapsed = now - startTime;
        state = STOPPED;
      }

      // automatisch stoppen wanneer reverse tijd voorbij is
      if (now - startTime >= elapsed) {
        elapsed = 0;       // reset zodat volgende druk weer FORWARD start
        state = STOPPED;
      }

      break;
  }
}
