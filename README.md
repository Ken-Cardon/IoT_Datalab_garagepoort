# IoT Datalab Garagepoort KenC

## Deploy steps:
-update esp met nieuwe code met mqtt + testen
-Python container blijft herstarten..
-sensor pi testen ter plaatse (op andere microcontroller?)

-NodeRed debuggen
-> nazicht nodes en datastroom syntax
-Mqtt debuggen
-> luistert extern op poort 1884 (waarom werkte dit wel bij iot-dashboard?)
-Pangolin /Traefik debuggen

-Verslag maken




## Default logins
Token InfluxDB
-> regel in yml, token file in folder en token zelf bewaard.. ?

-docker-compose.yml
-> regel paswoord voor Traefik uitgehashed
-> waar stel je het paswoord in van Traefik?

-mosquitto.conf
-> anonieme login en paswoordregel voorlopig uitgehashed

-inhoud setup.sh
-> htpasswd en passwd files aanmaken via script?
-> user en paswoord mqtt verwijderd vanwege authentication failure
-> user en paswoord NodeRed via web 
-> in tegenstelling tot InfluxDB en Grafana in yml en mqtt in .sh en code
