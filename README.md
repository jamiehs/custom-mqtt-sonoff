# MQTT Sonoff Switch Implementation

For Sonoff 10A inline switch

## Be Careful

**Sonoff devices switch mains power.  
Do not use this guide or mess with mains voltage without the proper precautions.**


### Based on the following libraries

* [PubSubClient](https://github.com/knolleary/pubsubclient)
* [ESP8266WiFi, DNSServer](https://github.com/esp8266/Arduino)
* [WiFiManager](https://github.com/tzapu/WiFiManager)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

We stand on the shoulders of giants when we hack things together using libraries like the above ones. Thank you!


## Features

* No hard-coded credentials thanks to WiFiManager:
  * WiFi password
  * Device slug
  * MQTT host
  * MQTT port
  * MQTT username
  * MQTT password
* Powers on by default once connected to WiFi.
* Green light shows if switch is on or off.
* Controllable via MQTT only.


## Setup

1. Flash firmware onto your Sonoff using [this guide](https://www.youtube.com/watch?v=-JxPWA-qxAk).
1. Power up the Sonoff by wiring it up correctly.
1. Choose the open ESPXXXXX WiFi access point.
1. Cick the "Sign into WiFi" notification or navigate to `192.168.4.1`.
1. Choose your WiFi network and enter the shared key (password).
1. Enter a name (all lowercase, underscores OK) for the Sonoff.
1. Enter your MQTT server details.


## Topics

**Subscribed to:**  
`sonoff/your_slug`

**Publishes:**  
`sonoff/your_slug/status`

The payload of both topics should be either `ON` or `OFF`


## Resources

* [Home Assistant](https://home-assistant.io/)
* [Mosquitto](http://mosquitto.org/) [BRUH Tutorial](https://www.youtube.com/watch?v=AsDHEDbyLfg)
* [MyMQTT (Android)](https://play.google.com/store/apps/details?id=at.tripwire.mqtt.client&hl=en) Subscribe to `#` in order to see all the MQTT chatter.
