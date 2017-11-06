#include <FS.h>
#include <PubSubClient.h>        // https://github.com/knolleary/pubsubclient
#include <ESP8266WiFi.h>         // https://github.com/esp8266/Arduino
#include <DNSServer.h>           // https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>         // https://github.com/bblanchon/ArduinoJson

// Pin definitions
#define RELAY_PIN 12
#define LED_PIN 13
#define BUTTON_PIN 0

// Configurable and SPIFFS save-able items
char device_slug[40];
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_user[40];
char mqtt_password[40];

// Tracks if we should be trying to save data
bool should_save_config = false;

// Get the mac address and default topic fragment
String mac = WiFi.macAddress();
String topic = "sonoff";

// Stores the previous state of the LED when blinking during long-press
byte led_pin_previous = HIGH;

// Initializes the MQTT reconnect timer.
unsigned long previous_mqtt_reconnect_millis = 0;

// Button code for determining if it's a toggle press or a reset press
// This uses a timer to see how long a button has been pressed
int button_pin_current;             // Current state of the button
long button_pin_millis_held;        // How long the button was held (milliseconds)
long button_pin_secs_held;          // How long the button was held (seconds)
long button_pin_prev_secs_held;     // How long the button was held in the previous check
byte button_pin_previous = HIGH;
unsigned long button_pin_firstTime; // how long since the button was first pressed

// Initialize WiFi Connectivity, MQTT, WiFi Manager
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiManager wifiManager;

// Callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  should_save_config = true;
}

// Handler for the incoming MQTT topics and payloads
void mqttCallback(char* incomingTopic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String strTopic = String((char*)incomingTopic);
  String msg = String((char*)payload);

  Serial.println("----------------------");
  Serial.println(strTopic);
  Serial.println(msg);
  Serial.println("----------------------");

  if (strTopic == topic) {
    if(msg == "ON"){
      powerOn();
    }
    if(msg == "OFF"){
      powerOff();
    }
  }
}

// Reconnect to MQTT if the connection fails
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  // If you do not want to use a username and password, change next line to
  // if (mqttClient.connect("ESP8266Client")) {
  if (mqttClient.connect(mac.c_str(), mqtt_user, mqtt_password)) {
    Serial.println("connected");
    // Subscribe to our globally defined topic
    mqttClient.subscribe(topic.c_str());
    // Say Hello
    mqttClient.publish(String(topic + "/status").c_str(), String("BOOTED").c_str(), true);
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 15 seconds");
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Enable and turn off!
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // HIGH is off

  //clean FS, for testing
  // SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");
  Serial.print("MAC Address: ");
  Serial.println(mac);

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(device_slug, json["device_slug"]);
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_password, json["mqtt_password"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read


  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_device_slug("device_slug", "Device Slug", device_slug, 40);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT User", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqtt_password, 40);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_device_slug);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);

  //reset settings - for testing
  // wifiManager.resetSettings();

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(device_slug, custom_device_slug.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());
  topic = topic + "/" + String(device_slug);

  //save the custom parameters to FS
  if (should_save_config) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["device_slug"] = device_slug;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  // MQTT Setup & Callback
  mqttClient.setServer(mqtt_server, atol(mqtt_port));
  mqttClient.setCallback(mqttCallback);

  // Power on once connected to WiFi
  powerOn();
}

void loop() {
  button_pin_current = digitalRead(BUTTON_PIN);

  // Restore LED previous state on release
  if (button_pin_current == HIGH && button_pin_previous == LOW) {
    digitalWrite(LED_PIN, led_pin_previous);
  }

  // If the button state changes to pressed, remember the start time
  if (button_pin_current == LOW && button_pin_previous == HIGH && (millis() - button_pin_firstTime) > 200) {
    button_pin_firstTime = millis();
    led_pin_previous = digitalRead(LED_PIN);
  }

  button_pin_millis_held = (millis() - button_pin_firstTime);
  button_pin_secs_held = button_pin_millis_held / 1000;

  // If the button was held down for a significant amount of time
  if (button_pin_millis_held > 50) {

    // If button is being held, then flash it on and off every second
    if (button_pin_current == LOW && button_pin_secs_held > button_pin_prev_secs_held) {
      Serial.println("Tick");
      // Toggle Light with each tick
      if (digitalRead(LED_PIN) == HIGH) {
        digitalWrite(LED_PIN, LOW);
      } else {
        digitalWrite(LED_PIN, HIGH);
      }
    }

    // Check if the button was released since we last checked
    if (button_pin_current == HIGH && button_pin_previous == LOW) {
      if (button_pin_secs_held <= 0) {
        Serial.println("Toggle");
        if (digitalRead(RELAY_PIN) == HIGH) {
          powerOff();
        } else {
          powerOn();
        }
      }
      if (button_pin_secs_held >= 10 && button_pin_secs_held < 180) {
        Serial.println("Attempting to clear flash...");
        delay(1000);

        Serial.println("clearing SPIFFS...");
        SPIFFS.format();
        delay(3000);

        Serial.println("resetting WiFi settings...");
        wifiManager.resetSettings();
        delay(3000);

        Serial.println("rebooting...");
        ESP.reset();
        delay(5000);
      }
    }
  }

  button_pin_previous = button_pin_current;
  button_pin_prev_secs_held = button_pin_secs_held;

  if (!mqttClient.connected()) {
    unsigned long current_mqtt_reconnect_millis = millis();

    if (current_mqtt_reconnect_millis - previous_mqtt_reconnect_millis >= 15000) {
      previous_mqtt_reconnect_millis = current_mqtt_reconnect_millis;
      reconnect();
    }
  }
  mqttClient.loop();
}

void powerOn() {
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  statusReport("ON");
}

void powerOff() {
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);
  statusReport("OFF");
}

void statusReport(String status) {
  mqttClient.publish(String(topic + "/status").c_str(), status.c_str(), true);
}