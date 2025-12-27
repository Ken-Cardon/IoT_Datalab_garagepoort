#!/bin/bash

if [ "$USER" != "root" ]; then
    echo "use sudo! exiting now"
    exit 1
fi

apt update
apt install -y mosquitto mosquitto-clients apache2-utils

echo "--- Mosquitto user setup ---"
read -p "MQTT username: " mqtt_user
mosquitto_passwd -c passwd_mqtt "$mqtt_user"
chown root:root passwd_mqtt
chmod 600 passwd_mqtt
echo "Mosquitto passwd file created."

echo "--- Web (Traefik/Node-RED) user setup ---"
read -p "Web username: " web_user
htpasswd -c passwd_web "$web_user"
chown root:root passwd_web
chmod 600 passwd_web
echo "Web passwd file created."

mkdir -p grafana_data
chown 472:472 grafana_data
chmod 755 grafana_data

mkdir -p node_red_data
chown 1000:1000 node_red_data
chmod 755 node_red_data

echo "Setup complete."
