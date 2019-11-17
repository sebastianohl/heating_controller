
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

#include "config.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include "TemperatureSensor.h"

#include <vector>

using namespace std;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorsLib(&oneWire);
vector<TemperatureSensor*> sensors;

WiFiClient espClient;
WiFiMulti WiFiMulti;

PubSubClient mqttClient(espClient);

// LED Pin
void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttRecvCallback);

  //pinMode(ledPin, OUTPUT);

  // Start up the library
  sensorsLib.begin();

  TemperatureSensor::constructDevices(sensors, sensorsLib, mqttClient);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);

  WiFiMulti.addAP(wifiSSID, wifiPassword);

  while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttRecvCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqttID, mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Subscribe
      for(int i=0;i<sensors.size(); i++){
         sensors[i]->run();
      }
      mqttClient.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  static long lastMsg = 0;
  if (!mqttClient.connected()) {
  	mqttReconnect();
  }
  mqttClient.loop();

  long now = millis();
  char str[100];
  if (now - lastMsg > 5000) {
    lastMsg = now;

	  // Loop through each device, print out temperature data
	  for(int i=0;i<sensors.size(); i++){
	      sensors[i]->run();
	    // Search the wire for address
	      // Output the device ID
	      Serial.print("Temperature for device: ");
	      Serial.println(i,DEC);
	      // Print the data
	      float tempC = sensors[i]->getCurrentTemperature();
	      Serial.print("Temp C: ");
	      sprintf(str, "%.2f", tempC);
	      Serial.println(tempC);
	  }
    
  }
}


