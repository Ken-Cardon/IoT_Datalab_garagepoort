#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

int led_pin = 0;
int druk_pin = 4;

bool isRunning = false;
bool isForward = true;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;

const unsigned long MAX_DURATION = 10000;

// variabelen WIFI
const char WIFI_SSID[] = "Bletchley";
const char WIFI_PASSWORD[] = "laptop!internet";

// variabelen MQTT
const char MQTT_HOST[] = "10.150.242.103";
const int  MQTT_PORT = 1883;
const char MQTT_USER[] = "";
const char MQTT_PASSWORD[] = "";

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT publish
void mqttSend(const char* topic, const String& payload) {
  if (client.connected()) {
    client.publish(topic, payload.c_str());
  }
}

// Connect WiFi
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Connect MQTT
void connectMQTT() {
  while (!client.connected()) {
  client.connect("Garagepoort_pi");
  client.connect("Garagepoort_pi", MQTT_USER, MQTT_PASSWORD);
    delay(500);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(druk_pin, INPUT); // externe pulldown
  digitalWrite(led_pin, LOW); // led uit bij start

  WiFi.mode(WIFI_STA);
  connectWiFi();

  client.setServer(MQTT_HOST, MQTT_PORT);
  connectMQTT();
}

void loop() {
  client.loop();

  bool pressed = digitalRead(druk_pin);

  if (pressed) {
    mqttSend("garagepoort/knop", "ingedrukt");

    if (!isRunning) {
      // poort openen
      isRunning = true;
      startTime = millis();

      elapsedTime = constrain(elapsedTime, 0, MAX_DURATION);
      digitalWrite(led_pin, HIGH);

      mqttSend("garagepoort/status", isForward ? "gaat open" : "gaat dicht");
      mqttSend("garagepoort/voortgang", String(elapsedTime));
    }
    else {
      // stoppen
      isRunning = false;
      elapsedTime = millis() - startTime;

      digitalWrite(led_pin, LOW);

      mqttSend("garagepoort/status", "gestopt");
      mqttSend("garagepoort/voortgang", String(elapsedTime));

      isForward = !isForward;   // richting omkeren voor de volgende keer
    }

    delay(300); // debounce
  }

  if (isRunning) {
    unsigned long currentElapsed = millis() - startTime;

    mqttSend("garagepoort/voortgang", String(currentElapsed));

    if (currentElapsed >= MAX_DURATION) {
      isRunning = false;
      elapsedTime = 0;
      digitalWrite(led_pin, LOW);

      mqttSend("garagepoort/status", isForward ? "geopend" : "gesloten");
    }
  }
}
