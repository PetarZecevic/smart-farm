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

class UpdateFunction
{
public:
	virtual void operator()(rapidjson::Document& state, rapidjson::Document& newState) = 0;	
};

class MQTT_Client
{
public:
    using message_handler = FP<void, MQTT::MessageData&>;
    MQTT_Client(const IPInfo& info, std::string logFileName);
    /**
     * Set user and gateway id from ssdp response. 
     * Set topics for communication with gateway.
     */
    void setLog(std::string ssdpLog);
    bool connectToBroker(std::string brokerLocation, int port);
    bool subscribe();
    /**
     * Send device description to gateway log topic.
     */
    bool sendInfo();
    /**
     * Send measured parameter values to gateway.
     */
    void report(ReportFunction* repFunc);
    /**
     * Set update function that will be called after
     * gateway sends new state to device on /update topic.
     * It is recommended to set this before report loop.
     */
    void setUpdateFunction(UpdateFunction* updateFunc) {updateFunction_ = updateFunc;}
    void waitFor(int milliseconds);
    bool isReportAllowed() {return reportAllowed_;};
    void recordLog(const std::string& logMessage);
    ~MQTT_Client();
private:
    bool sendReport(rapidjson::Document& state);
    // Callback methods for each of topic methods /get,/update,/log
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
    UpdateFunction* updateFunction_;
};

#endif // MQTT_CLIENT_HPP
