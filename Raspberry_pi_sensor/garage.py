import board
import adafruit_dht
import paho.mqtt.client as mqtt
import time

# --- CONFIGURATIE ---
#MQTT_HOST = "10.150.242.103"
MQTT_HOST = "192.168.0.123"
MQTT_PORT = 1883
TOPIC_BASE = "garage/"
PUBLISH_INTERVAL = 30

#sensor op GPIO14
dht_device = adafruit_dht.DHT11(board.D14)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("MQTT verbinding succesvol.")
    else:
        print(f"MQTT verbinding mislukt met code {rc}.")

def read_sensor_data():
    try:
        temperature = dht_device.temperature
        humidity = dht_device.humidity

        if temperature is not None and humidity is not None:
            return round(temperature, 1), round(humidity, 1)
    except RuntimeError as error:
        print(f"Systeem melding: {error.args[0]}")
    except Exception as e:
        print(f"Fout bij uitlezen: {e}")

    return None, None

def main():
    client = mqtt.Client(client_id="RPI_Garage_Sensor")
    client.on_connect = on_connect

    print(f"Verbinden met MQTT-broker op {MQTT_HOST}...")

    try:
        client.connect(MQTT_HOST, MQTT_PORT, 60)
        client.loop_start()

        while True:
            temp, hum = read_sensor_data()

            if temp is not None and hum is not None:
                client.publish(TOPIC_BASE + "temperatuur", str(temp), qos=1)
                client.publish(TOPIC_BASE + "vochtigheid", str(hum), qos=1)
                print(f"Gepubliceerd: T={temp}°C, H={hum}%")
            else:
                print("⚠️on geen sensorwaarden ophalen, volgende poging over 30s.")

            time.sleep(PUBLISH_INTERVAL)

    except KeyboardInterrupt:
        print("\nScript gestopt door gebruiker.")
    finally:
        dht_device.exit()
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
