/**
This code borrows from this following:
debounce:   https://www.arduino.cc/en/tutorial/debounce
publishing: https://github.com/Nilhcem/home-monitoring-grafana/blob/master/03-bme280_mqtt/esp8266/esp8266.ino
**/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_TOPIC_METER "home/irrigation_meter/gpm"
#define MQTT_TOPIC_STATE "home/nodemcu1/status"
#define MQTT_CLIENT_ID "nodemcu1"

const int meterPin = 16;                   
const int PUBLISH_DELAY = 5000;
const int pulseDelay = 1000;              // ms; how long to wait for pulse to finish
const unsigned long debounceDelay = 50;   // ms
const char *WIFI_SSID = "Wardians2.0";
const char *WIFI_PASSWORD = "21jan2012";
const char *MQTT_SERVER = "10.83.83.53";
const int MQTT_PORT = 1883;
const char *MQTT_USER = "NULL"; // NULL for no authentication
const char *MQTT_PASSWORD = "NULL"; // NULL for no authentication

int meterState;
int lastMeterState = HIGH;
int gallonsUsed = 0;
unsigned int lastDebounceTime = 0;
unsigned long lastMsgTime = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(9600);  
  while (! Serial);
  Serial.println('begin');
  pinMode(meterPin, INPUT_PULLDOWN_16);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  long now = millis();
  
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
  
  if (now - lastMsgTime > PUBLISH_DELAY) {
    lastMsgTime = now;
    Serial.println(gallonsUsed);
    mqttPublish(MQTT_TOPIC_METER, gallonsUsed);
    gallonsUsed = 0;
  }
  int reading = digitalRead(meterPin);
  
  if (reading != lastMeterState) {
     lastDebounceTime = millis(); 
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (meterState != reading) {
      meterState = reading;
    }
      
    if (meterState == HIGH) {
      gallonsUsed++;
      delay(pulseDelay);
    }
  }
      
  lastMeterState = reading;
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttPublish(char *topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
