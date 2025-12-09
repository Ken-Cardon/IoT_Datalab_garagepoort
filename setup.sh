#!/bin/bash


if [ $USER != "root" ]
then
    echo "use sudo! exiting now"
    exit
fi

apt update
apt install -y mosquitto mosquitto-clients

mkdir -p influx_conf

echo "--- Mosquitto Beveiliging Setup ---"
read -p "GEBRUIKERSNAAM?" mqtt_user
if [ -z "$mqtt_user" ]; then
    echo "Geen gebruikersnaam ingevoerd. Setup afgebroken."
    exit 1
fi

echo "WACHTWOORD?"
mosquitto_passwd -c passwd "$mqtt_user"

chown mosquitto:mosquitto passwd
chmod 600 passwd

echo "succes"
