import paho.mqtt.client as mqtt
import time
import random
import json # Handig voor het verzenden van één gecombineerd JSON-bericht


MQTT_HOST = "10.150.242.103"
MQTT_PORT = 1884
#MQTT_USER = "KenC"
#MQTT_PASSWORD = "KenCMQTT"
TOPIC_BASE = "garage/"

# Tijd tussen metingen (in seconden)
PUBLISH_INTERVAL = 30

# MQTT communicatie

def on_connect(client, userdata, flags, rc):

    if rc == 0:
        print("MQTT verbinding succesvol.")
    else:
        print(f"MQTT verbinding mislukt met code {rc}. Opnieuw proberen...")

def on_disconnect(client, userdata, rc):

    print("MQTT verbinding verbroken.")

def read_sensor_data():
  # sensordata inlezen
    try:
        # Lees data en wacht 2 seconden zoals vereist door DHT11
        temperature = dht_device.temperature
        humidity = dht_device.humidity

        if temperature is not None and humidity is not None:
            # We ronden af voor de netheid, hoewel de DHT11 vaak integers levert
            return round(temperature, 1), round(humidity, 1)

        else:
            print("Kon geen geldige data van de DHT-sensor lezen.")
            return None, None
    except RuntimeError as error:
        # Leesfouten treden op als we te snel lezen
        print(f"Runtime fout bij het lezen van de DHT: {error.args[0]}")
        return None, None
    except Exception as e:
        print(f"Andere fout bij het lezen van sensordata: {e}")
        return None, None

def main():
  # Initialiseert de MQTT-client en start de publicatie.
    client = mqtt.Client(client_id="RPI_TempHumi_Sensor")
   #client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

    print(f"Verbinden met MQTT-broker op {MQTT_HOST}:{MQTT_PORT}...")

    try:
        client.connect(MQTT_HOST, MQTT_PORT, 60)
        client.loop_start() # Start een aparte thread om netwerkverkeer af te handelen

        while True:
            temperature, humidity = read_sensor_data()

            if temperature is not None and humidity is not None:

                client.publish(TOPIC_BASE + "temperatuur", str(temperature), qos=1)
                client.publish(TOPIC_BASE + "vochtigheid", str(humidity), qos=1)

                print(f"Gepubliceerd: T={temperature}°C, H={humidity}%.")

            time.sleep(PUBLISH_INTERVAL)

    except KeyboardInterrupt:
        print("\nAfgesloten door gebruiker.")
    except Exception as e:
        print(f"Er is een onverwachte fout opgetreden: {e}")
    finally:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
