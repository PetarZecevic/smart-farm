#ifndef MQTT_MANAGER_HPP
#define MQTT_MANAGER_HPP

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <mqtt/async_client.h>
#include "device.hpp"

class MQTT_Manager
{
public:
    MQTT_Manager(std::string userId, std::string gatewayId, std::string brokerAddress);
    bool connectToBroker(mqtt::connect_options coptions);
    bool start();
    void registerDevice(std::string id, rapidjson::Document& info, rapidjson::Document& state);
    void unregisterDevice(std::string id);
    void mergeDeviceState(std::string id, const rapidjson::Document& state);
    std::string getUserId() const {return user_id_;};
    std::string getGatewayId() const {return gateway_id_;};
    std::string getAllInfo();
    std::string getAllState();
    std::string getOneInfo(std::string id);
    std::string getOneState(std::string id);
    // std::string getDeviceState(std::string deviceId);
    // std::string getDeviceInfo(std::string deviceId);
    //std::unordered_map<std::string, Device>& getDevices() { return devices_;};
    /*
    Close log file if it is opened.
    */
    ~MQTT_Manager();
private:

    /*
    Write string to file for log informations.
    Log is defined by template:
        time -> event
        time is hour:min:second.
    */
    void recordLog(const std::string& logMessage);
    std::string user_id_;
    std::string gateway_id_;
    std::string broker_addr_;
    mqtt::async_client mqtt_client_;
    /**
     * Local callback class for use with the client connection.
     * This is primarily intended to receive messages.
     */
    class callback : public virtual mqtt::callback,
                        public virtual mqtt::iaction_listener
    {
        // The MQTT client
        mqtt::async_client& cli_;
        // The MQTT manager
        MQTT_Manager& manager_;
        // Manager topic for device logging.
        std::string managerLog_;
        // Re-connection failure
        void on_failure(const mqtt::token& tok) override;
        // (Re)connection success
        // Either this or connected() can be used for callbacks.
        void on_success(const mqtt::token& tok) override {}
        // (Re)connection success
        void connected(const std::string& cause) override;
        // Callback for when the connection is lost.
        void connection_lost(const std::string& cause) override;
        // Callback for when a message arrives.
        void message_arrived(mqtt::const_message_ptr msg) override;
        void delivery_complete(mqtt::delivery_token_ptr token) override {}
        // Parsing function.
        void split(const std::string& s, char delimiter, std::vector<std::string>& tokens);
    public:
        callback(mqtt::async_client& cli, MQTT_Manager& manager): 
            cli_(cli),
            manager_(manager)
        {
            managerLog_ = manager_.getUserId() + "/gateway/" + manager_.getGatewayId() + "/log";
        }
    };
    callback cb_;
    std::unordered_map<std::string, Device> devices_;
    std::fstream logFile_;
};

#endif // MQTT_MANAGER_HPP