#include "mqtt_client.hpp"
#include <iostream>

static int arrived = 0;

// Public
MQTT_Client::MQTT_Client(IPInfo_t info):
    ipinfo(info),
    ipstack(),
    client(ipstack)
{
    iplog = std::string("");
    gatewaylog = std::string("");
}

void MQTT_Client::setLog(std::string ssdp_log)
{
    gatewaylog = ssdp_log;
    int pos = gatewaylog.find_first_of('/', 0);
	iplog = gatewaylog.substr(0, pos);
	iplog += "/" + ipinfo.group + "/" + ipinfo.id + "/log";
}

bool MQTT_Client::connectToBroker(std::string brokerLocation, int port)
{
    int rc = ipstack.connect(brokerLocation.c_str(), port);
	if (rc != MQTT::returnCode::SUCCESS)
	{
        return false;
	}
	else
	{
		printf("MQTT connecting\n");
    	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    	data.MQTTVersion = 3;
    	data.clientID.cstring = (char*)ipinfo.id.c_str();
		rc = client.connect(data);
		if (rc != MQTT::returnCode::SUCCESS)
		{
	    	return false;
		}
    }	
    return true;
}


void static logMessageCallback(MQTT::MessageData& mdata)
{
    MQTT::Message m = mdata.message;
    std::cout << "Received message on topic " << mdata.topicName.cstring << std::endl;
    std::cout << (char*)m.payload << std::endl;
    arrived++;  
}

bool MQTT_Client::sendInfo()
{
    int rc = client.subscribe(iplog.c_str(), MQTT::QOS2, logMessageCallback);   
	if (rc != MQTT::returnCode::SUCCESS)
	{
		return false;
	}
    else
    {
        // Send device information to gateway.
        std::string builder = "";
        builder += "group : " + ipinfo.group + "\n";
        builder += "id : " + ipinfo.id;
        rc = client.publish(gatewaylog.c_str(), (void*)builder.c_str(), builder.length(), MQTT::QOS2);
        if(rc != MQTT::returnCode::SUCCESS)
            return false;
        while (arrived == 0)
            client.yield(100);
    }
    return true;
}