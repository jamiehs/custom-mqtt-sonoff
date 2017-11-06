# MQTT Sonoff Switch Implementation

For Sonoff 10A inline switch (Sonoff Basic)

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
* Controllable via MQTT.
* Toggle on/off state with the push button.
* Reset all credentials by holding the button for approximately 10 flashes.
* Tolerant of both WiFi and MQTT server interruptions.


## Setup

1. Install the above libraries into your Arduino IDE.
1. Flash firmware onto your Sonoff using [this guide](https://www.youtube.com/watch?v=-JxPWA-qxAk).
1. Power up the Sonoff by wiring it up correctly.
1. Choose the open ESPXXXXX WiFi access point.
1. Click the "Sign into WiFi" notification or navigate to `192.168.4.1`.
1. Choose your WiFi network and enter the shared key (password).
1. Enter a name (all lowercase, underscores OK) for the Sonoff.
1. Enter your MQTT server details.

### My Flashing Settings (may vary for your module or USB-serial interface)

In addition to the above video, [arendst](https://github.com/arendst) has [a few good annotated photos](https://github.com/arendst/Sonoff-Tasmota/wiki/GPIO-Locations#sonoff-basic) that show the pins for flashing and the GPIOs too.

* Board: **Generic ESP8266 Module**
* Flash Mode: **DIO**
* Flash Frequency: **40MHz**
* CPU Frequency: **80MHz**
* Flash Size: **512K (64K SPIFFS)**
* Upload Speed: **115200**


### Troubleshooting

* If the flash fails, try swapping the TX and RX lines.
* When flashing from Arduino IDE if the upload fails before progress is displayed, you may need to attempt a second flash; sometimes the first attempt will fail for no apparent reason.
* You must manually put the module into flash mode by holding the button (GPIO 0) when powering it up. This must be done each time you flash it.


## Topics

**Subscribed to:**  
`sonoff/[your slug]`

**Publishes:**  
`sonoff/[your slug]/status`

The payload of both topics should be either `ON` or `OFF`


## Learning Resources

* [Home Assistant](https://home-assistant.io/)
* [Mosquitto](http://mosquitto.org/) setup via [BRUH Tutorial](https://www.youtube.com/watch?v=AsDHEDbyLfg)
* [MyMQTT (Android)](https://play.google.com/store/apps/details?id=at.tripwire.mqtt.client&hl=en) Subscribe to `#` in order to see all the MQTT chatter.
* [Button Press Code](https://playground.arduino.cc/Code/HoldButton) from [playground.arduino.cc](https://playground.arduino.cc)
