#ifndef TEMPERATURE_SENSOR_H_
#define TEMPERATURE_SENSOR_H_
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include <string.h>
#include <vector>

class TemperatureSensor
{
public:
    TemperatureSensor(DallasTemperature &sensorsLib, const DeviceAddress &address, PubSubClient &mqttClient):
	    m_sensorsLib(sensorsLib),
	    m_mqttClient(mqttClient)
	{
		memcpy(m_address, address, sizeof(address));
	}

     void run()
     {
	// Search the wire for address
	if(m_sensorsLib.isConnected(m_address)) {
		float value = getCurrentTemperature();
		m_mqttClient.publish("esp32/alive", "foo");
	}
     }

     const float &getCurrentTemperature()
     {
	// Search the wire for address
	if(m_sensorsLib.isConnected(m_address)) {
		m_sensorsLib.requestTemperaturesByAddress(m_address);
		float value = m_sensorsLib.getTempC(m_address);
		return value;
	}
        return 0;
     }

     const DeviceAddress &getAddress()
     {
	     return m_address;
     }

     // function to print a device address
     void printAddress() {
  	for (uint8_t i = 0; i < 8; i++){
    	    if (m_address[i] < 16) Serial.print("0");
      	    Serial.print(m_address[i], HEX);
  	}
     }

     static void constructDevices(std::vector<TemperatureSensor *> &sensors, DallasTemperature &sensorsLib, PubSubClient &mqttClient)
     {
	  // Grab a count of devices on the wire
	  int numberOfDevices = sensorsLib.getDeviceCount();
	  
	  // locate devices on the bus
	  Serial.print("Locating devices...");
	  Serial.print("Found ");
	  Serial.print(numberOfDevices, DEC);
	  Serial.println(" devices.");

	  // Loop through each device, print out address
	  for(int i=0;i<numberOfDevices; i++){
	    DeviceAddress tempDeviceAddress;
	    // Search the wire for address
	    if(sensorsLib.getAddress(tempDeviceAddress, i)){
	      TemperatureSensor *t = new TemperatureSensor(sensorsLib, tempDeviceAddress, mqttClient);
	      sensors.push_back(t);
	      Serial.print("Found device ");
	      Serial.print(i, DEC);
	      Serial.print(" with address: ");
	      t->printAddress(); 
	      Serial.println();
	    } else {
	      Serial.print("Found ghost device at ");
	      Serial.print(i, DEC);
	      Serial.print(" but could not detect address. Check power and cabling");
	    }
	  }
     }

private:
      DallasTemperature &m_sensorsLib;
      DeviceAddress m_address; 
      PubSubClient &m_mqttClient;
};

#endif
