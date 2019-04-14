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

// Public
MQTT_Client::MQTT_Client(IPInfo_t info):
    ipinfo_(info),
    ipstack_(),
    client_(ipstack_)
{
    iplog_ = std::string("");
    gatewaylog_ = std::string("");
}

void MQTT_Client::setLog(std::string ssdp_log)
{
    gatewaylog_ = ssdp_log;
    int pos = gatewaylog_.find_first_of('/', 0);
	iplog_ = gatewaylog_.substr(0, pos);
	iplog_ += "/" + ipinfo_.group + "/" + ipinfo_.id + "/log";
}

bool MQTT_Client::connectToBroker(std::string brokerLocation, int port)
{
    int rc = ipstack_.connect(brokerLocation.c_str(), port);
	if (rc != MQTT::returnCode::SUCCESS)
	{
        return false;
	}
	else
	{
		printf("MQTT connecting\n");
    	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    	data.MQTTVersion = 3;
    	data.clientID.cstring = (char*)ipinfo_.id.c_str();
		rc = client_.connect(data);
		if (rc != MQTT::returnCode::SUCCESS)
		{
	    	return false;
		}
    }	
    return true;
}

void MQTT_Client::logCallback(MQTT::MessageData& mdata)
{
    MQTT::Message &message = mdata.message;

    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
}

bool MQTT_Client::sendInfo()
{
    int rc = client_.subscribe(iplog_.c_str(), MQTT::QOS2,
        message_handler(this, &MQTT_Client::logCallback));
       
	if (rc != MQTT::returnCode::SUCCESS)
	{
		return false;
	}
    else
    {
        // Send device information to gateway.
        std::string builder = "";
        builder += ipinfo_.group + "/" + ipinfo_.id;
        rc = client_.publish(gatewaylog_.c_str(), (void*)builder.c_str(), builder.length(), MQTT::QOS2);
        if(rc != MQTT::returnCode::SUCCESS)
            return false;
        client_.yield(100);
    }
    return true;
}