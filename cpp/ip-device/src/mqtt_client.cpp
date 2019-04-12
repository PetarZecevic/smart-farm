#include "mqtt_client.hpp"
#include <cstdio>

/*
class CallbackFunction
{
public:
    virtual void operator()(MQTT::MessageData& data) = 0;
};

class LogCallback : public virtual CallbackFunction
{
public:
    void operator()(MQTT::MessageData& mdata) override
    {
        MQTT::Message &message = mdata.message;
        printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", 
            message.qos, message.retained, message.dup, message.id);
        printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
        arrived++;
    }
};
*/

int MQTT_Client::arrived = 0;

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

/*
void static logMessageCallback(MQTT::MessageData& mdata)
{
    MQTT::Message &message = mdata.message;

    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
    arrived++;
}
*/

void MQTT_Client::logCallback(MQTT::MessageData& mdata)
{
    MQTT::Message &message = mdata.message;

    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
    arrived++;
}

bool MQTT_Client::sendInfo()
{
    int rc = client.subscribe(iplog.c_str(), MQTT::QOS2,
        message_handler(this, &MQTT_Client::logCallback));
       
	if (rc != MQTT::returnCode::SUCCESS)
	{
		return false;
	}
    else
    {
        // Send device information to gateway.
        std::string builder = "";
        builder += ipinfo.group + "/" + ipinfo.id;
        rc = client.publish(gatewaylog.c_str(), (void*)builder.c_str(), builder.length(), MQTT::QOS2);
        if(rc != MQTT::returnCode::SUCCESS)
            return false;
        client.yield(100);
    }
    return true;
}