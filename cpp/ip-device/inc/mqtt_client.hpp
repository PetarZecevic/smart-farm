#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <unordered_map>
#define MQTTCLIENT_QOS2 1
#include <memory.h>
#include "mqtt-embedded/MQTTClient/MQTTClient.h"
#define DEFAULT_STACK_SIZE -1
#include "mqtt-embedded/MQTTClient/linux.cpp"
#include "ipinfo.hpp"

class MQTT_Client
{
public:
    using message_handler = FP<void, MQTT::MessageData&>;
    MQTT_Client(IPInfo& info);
    void setLog(std::string ssdpLog);
    bool connectToBroker(std::string brokerLocation, int port);
    bool subscribe();
    bool sendInfo();
    void waitFor(int milliseconds);
    bool isReportAllowed() {return reportAllowed_;};
private:
    void logCallback(MQTT::MessageData& mdata);
    void getCallback(MQTT::MessageData& mdata);
    void updateCallback(MQTT::MessageData& mdata);
    IPInfo ipinfo_;
    IPStack ipstack_;
    MQTT::Client<IPStack, Countdown> client_;
    std::string userid_;
    std::string gatewayid_;
    std::unordered_map<std::string, std::string> topics_;
    bool reportAllowed_;
};

#endif // MQTT_CLIENT_HPP