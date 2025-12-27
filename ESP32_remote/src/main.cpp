#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// PIN DEFINITIES
const int led_pin = 0;
const int druk_pin = 4;

// STATE VARIABELEN
bool isRunning = false;
bool isForward = true;
bool lastBtn = false; // Nodig voor non-blocking edge detection

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

// Herverbinding met de MQTT-broker
void reconnectMQTT() {
  // Loop tot we verbonden zijn
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Probeer te verbinden met clientID
    if (client.connect("Garagepoort_pi", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT connected.");
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wacht 5 seconden alvorens opnieuw te proberen
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(druk_pin, INPUT); // externe pulldown
  digitalWrite(led_pin, LOW); // led uit bij start

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
  reconnectMQTT();
}

void loop() {
  // Zorg dat de client verbonden blijft en berichten afhandelt
  if (!client.connected()) {
    reconnectMQTT(); // Probeer opnieuw te verbinden indien nodig
  }
  client.loop(); 

  // knop logica
  bool currBtn = digitalRead(druk_pin);
  bool pressed = (currBtn && !lastBtn); // enkel reactie op flank
  lastBtn = currBtn;

  unsigned long now = millis();

  if (pressed) {

    mqttSend("garagepoort/knop", "ingedrukt");

    if (!isRunning) {
    
      isRunning = true;
      startTime = now;
      digitalWrite(led_pin, HIGH); // start de motor

      if (elapsedTime == 0) {
        // volledig open of dicht geweest
        isForward = !isForward; // richting omdraaien
        elapsedTime = MAX_DURATION; // volledige looptijd
      }

      mqttSend("garagepoort/status", isForward ? "gaat open" : "gaat dicht");
      mqttSend("garagepoort/voortgang", String(elapsedTime));
    }
    else {
      // stoppen
      isRunning = false;
      unsigned long ran = now - startTime;
      
      // Bepaal de gelopen tijd en bewaar deze
      if (ran > elapsedTime) ran = elapsedTime;
      elapsedTime = ran;

      digitalWrite(led_pin, LOW); // stop de motor

      mqttSend("garagepoort/status", "gestopt");
      mqttSend("garagepoort/voortgang", String(elapsedTime));
    }

  }

  // auto stop na max looptijd
  if (isRunning) {
    unsigned long currentElapsed = now - startTime;
    
    if (currentElapsed >= elapsedTime) { // gebruik de bewaarde 'elapsedTime' als target duration
      
      // auto stop
      isRunning = false;
      elapsedTime = 0; // Volledig traject voltooid
      digitalWrite(led_pin, LOW);

      mqttSend("garagepoort/status", isForward ? "geopend" : "gesloten");
      mqttSend("garagepoort/voortgang", "0");
      
    }
  }
}