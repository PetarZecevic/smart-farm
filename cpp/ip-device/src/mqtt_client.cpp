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
MQTT_Client::MQTT_Client(IPInfo& info):
    ipstack_(),
    client_(ipstack_)
{
    ipinfo_.copy(info),
    userid_ = std::string("");
    gatewayid_ = std::string("");
    reportAllowed_ = false;
}

void MQTT_Client::setLog(std::string ssdpLog)
{
    // Parse user Id.
    int pos = ssdpLog.find_first_of('/', 0);
    userid_ += ssdpLog.substr(0, pos);

    // Parse gateway Id.
    int begin,end;
    begin = ssdpLog.find("gateway/");
    begin += 8;
    end  = ssdpLog.find("/log");
    gatewayid_ += ssdpLog.substr(begin, end-begin);
    
    // Create all topics for communication with MQTT broker.
	std::string iplog = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/log";
    std::string gatewaylog = userid_ + "/gateway/" + gatewayid_ + "/log";
    std::string report = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/report";
    std::string update = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/update";
    std::string get = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/get";
    
    // Insert topics into map.
    topics_.insert(std::make_pair("iplog", iplog));
    topics_.insert(std::make_pair("gatewaylog", gatewaylog));
    topics_.insert(std::make_pair("report", report));
    topics_.insert(std::make_pair("update", update));
    topics_.insert(std::make_pair("get", get));    
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
    	data.clientID.cstring = (char*)ipinfo_.getByKey("id").c_str();
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
    std::string response((char*)message.payload);
    printf("Response: %s\n", response.c_str());
    if(response == "OK")
    {
        printf("Log success!\n");
        // Subsribe to previously defined topics.
        client_.subscribe(topics_["update"].c_str(), MQTT::QOS1, message_handler(this, &MQTT_Client::updateCallback));
        client_.subscribe(topics_["get"].c_str(), MQTT::QOS1, message_handler(this, &MQTT_Client::getCallback));
        // Publish permission for reporting state parameters.
        reportAllowed_ = true;
    }  
    else
    {
        printf("Log failed!\n");
    }
}

void MQTT_Client::updateCallback(MQTT::MessageData& mdata)
{
    MQTT::Message& message = mdata.message;
    std::string update((char*)message.payload);
    printf("Parameters to update:\n");
    printf("\t%s\n", update.c_str());
}

void MQTT_Client::getCallback(MQTT::MessageData& mdata)
{
    MQTT::Message& message = mdata.message;
    std::string request((char*)message.payload);
    printf("Parameters to return:\n");
    printf("\t%s\n", request.c_str());
}

void MQTT_Client::waitFor(int milliseconds)
{
    client_.yield(milliseconds);
}

bool MQTT_Client::subscribe()
{
    int rc = client_.subscribe(topics_["iplog"].c_str(), MQTT::QOS2, message_handler(this, &MQTT_Client::logCallback));
       
	if (rc != MQTT::returnCode::SUCCESS)
		return false;
    else
        return true;
}

bool MQTT_Client::sendInfo()
{
    // Send device information to gateway.
    std::string builder = "";
    builder += ipinfo_.getDescriptionString();
    
    int rc = client_.publish(topics_["gatewaylog"].c_str(), (void*)builder.c_str(), builder.length(), MQTT::QOS2);
    
    if(rc != MQTT::returnCode::SUCCESS)
        return false;
    else
        return true;
}