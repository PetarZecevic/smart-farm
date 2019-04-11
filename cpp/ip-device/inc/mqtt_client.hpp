#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#define MQTTCLIENT_QOS2 1
#include <memory.h>
#include "mqtt-embedded/MQTTClient/MQTTClient.h"
#define DEFAULT_STACK_SIZE -1
#include "mqtt-embedded/MQTTClient/linux.cpp"

typedef struct IPInfo
{
	std::string id;
	std::string group;
}IPInfo_t;


class MQTT_Client
{
public:
    MQTT_Client(IPInfo_t info);
    void setLog(std::string ssdp_log);
    bool connectToBroker(std::string brokerLocation, int port);
    bool sendInfo();
private:
    //void logMessageCallback(MQTT::MessageData& mdata);
    IPInfo_t ipinfo;
    IPStack ipstack;
    MQTT::Client<IPStack, Countdown> client;
    std::string iplog;
    std::string gatewaylog;
};

#endif // MQTT_CLIENT_HPP