#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <unordered_map>
#include <fstream>
#define MQTTCLIENT_QOS2 1
#include <memory.h>
#include "mqtt-embedded/MQTTClient/MQTTClient.h"
#define DEFAULT_STACK_SIZE -1
#include "mqtt-embedded/MQTTClient/linux.cpp"
#include "ipinfo.hpp"


class ReportFunction
{
public:
    virtual bool operator()(rapidjson::Document& newState, rapidjson::Document& prevState) = 0;
};

class MQTT_Client
{
public:
    using message_handler = FP<void, MQTT::MessageData&>;
    MQTT_Client(const IPInfo& info, std::string logFileName);
    void setLog(std::string ssdpLog);
    bool connectToBroker(std::string brokerLocation, int port);
    bool subscribe();
    bool sendInfo();
    void report(ReportFunction* repFunc);
    void waitFor(int milliseconds);
    bool isReportAllowed() {return reportAllowed_;};
    void recordLog(const std::string& logMessage);
    ~MQTT_Client();
private:
    bool sendReport(rapidjson::Document& state);
    void logCallback(MQTT::MessageData& mdata);
    void getCallback(MQTT::MessageData& mdata);
    void updateCallback(MQTT::MessageData& mdata);
    IPStack ipstack_;
    MQTT::Client<IPStack, Countdown, 1000, 5> client_;
    IPInfo ipinfo_;
    std::string userid_;
    std::string gatewayid_;
    bool reportAllowed_;
    std::fstream logFile_;
    std::unordered_map<std::string, std::string> topics_;
};

#endif // MQTT_CLIENT_HPP