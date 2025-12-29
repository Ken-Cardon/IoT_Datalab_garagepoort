#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// PIN DEFINITIES
const int led_pin = 0;
const int druk_pin = 4;
const int led_groen = 2; // groene led toegevoegd

// STATE VARIABELEN
bool isRunning = false;
bool isForward = true;
bool lastBtn = false; // Nodig voor non-blocking edge detection
bool mqttTrigger = false; // om MQTT-knopdruk te detecteren

// TIMING VARIABELEN
unsigned long startTime = 0;
unsigned long elapsedTime = 0; // Tijd gelopen vóór stop (nodig voor REVERSE)
const unsigned long MAX_DURATION = 10000; // 10 seconden

// WIFI VARIABELEN
//const char WIFI_SSID[] = "bletchley";
//const char WIFI_PASSWORD[] = "laptop!internet";

const char WIFI_SSID[] = "Liesket";
const char WIFI_PASSWORD[] = "ahy8Akwk";

// MQTT VARIABELEN
//const char MQTT_HOST[] = "10.150.242.103";
const char MQTT_HOST[] = "192.168.0.123";
const int MQTT_PORT = 1883;
// nog in te stellen
const char MQTT_USER[] = "KenC";
const char MQTT_PASSWORD[] = "KenCMQTT";

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT publish
void mqttSend(const char* topic, const String& payload) {
  if (client.connected()) {
    // Print de actie naar Serial, handig voor debuggen
    Serial.print("MQTT → ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(payload);

    client.publish(topic, payload.c_str());
  }
}

// MQTT callback zodat virtuele knop werkt
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT ← ");
  Serial.print(t);
  Serial.print(": ");
  Serial.println(msg);

  if (t == "garagepoort/knop" && msg == "ingedrukt") {
    mqttTrigger = true; // zelfde gedrag als fysieke knop
  }
}

// Herverbinding met de MQTT-broker
void reconnectMQTT() {
  // Loop tot we verbonden zijn
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Probeer te verbinden met clientID
    if (client.connect("Garagepoort_pi", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT connected.");
      client.subscribe("garagepoort/knop"); // voor dashboard
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wacht 5 seconden alvorens opnieuw te proberen
      delay(5000);
    }
  }
}

void updateLeds() { // leds reageren enkel op status
  if (isRunning) {
    if (isForward) {
      digitalWrite(led_groen, HIGH);
      digitalWrite(led_pin, LOW);
    } else {
      digitalWrite(led_groen, LOW);
      digitalWrite(led_pin, HIGH);
    }
  } else {
    digitalWrite(led_groen, LOW);
    digitalWrite(led_pin, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(led_groen, OUTPUT); // voor dashboard
  pinMode(druk_pin, INPUT); // externe pulldown
  digitalWrite(led_pin, LOW); // led uit bij start
  digitalWrite(led_groen, LOW); // voor dashboard

  // WiFi verbinden
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MQTT server verbinden
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(mqttCallback); // voor dashboard
  reconnectMQTT();
}

void handlePress() { // centrale logica voor fysieke + virtuele knop

  unsigned long now = millis();

  if (!isRunning) {
    if (elapsedTime == 0) {
      // volledig open of dicht geweest
      isForward = !isForward; // richting omdraaien
      elapsedTime = MAX_DURATION; // volledige looptijd
    } else {
      // onderbroken → zelfde tijd teruglopen
      isForward = !isForward; // altijd omkeren bij herstart
    }

    isRunning = true;
    startTime = now;
    updateLeds(); // voor dashboard

    mqttSend("garagepoort/status", isForward ? "gaat open" : "gaat dicht");
    mqttSend("garagepoort/voortgang", String(elapsedTime));
  }
  else {
    // stoppen
    isRunning = false;
    unsigned long ran = now - startTime;
    
    if (ran > elapsedTime) ran = elapsedTime;
    elapsedTime = ran;

    updateLeds(); // voor dashboard

    mqttSend("garagepoort/status", "gestopt");
    mqttSend("garagepoort/voortgang", String(elapsedTime));
  }
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop(); 

  // fysieke knop stuurt enkel MQTT
  bool currBtn = digitalRead(druk_pin);
  bool pressed = (currBtn && !lastBtn);
  lastBtn = currBtn;

  if (pressed) {
    mqttTrigger = true; // fysieke knop triggert dezelfde logica
  }

  if (mqttTrigger) { // voor dashboard
    mqttTrigger = false;
    handlePress();
  }

  // auto stop na max looptijd
  if (isRunning) {
    unsigned long now = millis();
    unsigned long currentElapsed = now - startTime;
    
    if (currentElapsed >= elapsedTime) {
      isRunning = false;
      elapsedTime = 0;
      updateLeds(); // voor dashboard

      mqttSend("garagepoort/status", isForward ? "geopend" : "gesloten");
      mqttSend("garagepoort/voortgang", "0");
    }
  }
}
