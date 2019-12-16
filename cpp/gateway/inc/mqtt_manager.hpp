#ifndef MQTT_MANAGER_HPP
#define MQTT_MANAGER_HPP

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <mqtt/async_client.h>
#include "device.hpp"

/**
 * @brief Main component for handling IOT devices.
 * 
 */
class MQTT_Manager : public virtual mqtt::callback,
    public virtual mqtt::iaction_listener
{
    std::string user_id_;
    std::string gateway_id_;
    std::string broker_addr_;
    /* Library MQTT client used to handle communication with MQTT server */
    mqtt::async_client mqtt_client_;
    /* Manager topic for device logging */
    std::string managerLog_;
    /* Table that represents IOT devices connected to the gateway */
    std::unordered_map<std::string, Device> devices_;
    /* File used to write log informations */
    std::fstream logFile_;

public:
    /**
     * @brief Construct a new mqtt manager object
     * 
     * @param userId 
     * @param gatewayId 
     * @param brokerAddress 
     */
    MQTT_Manager(std::string userId, std::string gatewayId, std::string brokerAddress);
    
    /**
     * @brief Used for connecting to mqtt broker(server)
     * 
     * @param coptions 
     * @return true 
     * @return false 
     */
    bool connectToBroker(mqtt::connect_options coptions);
    
    /**
     * Subscribe to necessary topics, in order to enable 
     * communication with IOT devices on the network.
     * @return true 
     * @return false 
     */
    bool start();

    /**
     * @brief 
     * 
     * @param id 
     * @param info 
     * @param state 
     */
    void registerDevice(std::string id, rapidjson::Document& info, rapidjson::Document& state);
    
    /**
     * @brief 
     * 
     * @param id 
     */
    void unregisterDevice(std::string id);
    
    /**
     * @brief 
     * 
     * @param id 
     * @param state 
     */
    void mergeDeviceState(std::string id, const rapidjson::Document& state);
    
    /**
     * @brief Get the User Id object
     * 
     * @return std::string 
     */
    std::string getUserId() const {return user_id_;};
    
    /**
     * @brief Get the Gateway Id object
     * 
     * @return std::string 
     */
    std::string getGatewayId() const {return gateway_id_;};
    
    /**
     * @brief Get the All Devices Info object
     * 
     * @return std::string 
     */
    std::string getAllDevicesInfo();
    
    /**
     * @brief Get the All Devices State object
     * 
     * @return std::string 
     */
    std::string getAllDevicesState();
    
    /**
     * @brief Get the Device Info object
     * 
     * @param id 
     * @return std::string 
     */
    std::string getDeviceInfo(std::string id);
    
    /**
     * @brief Get the Device State object
     * 
     * @param id 
     * @return std::string 
     */
    std::string getDeviceState(std::string id);
    
    /**
     * @brief Destroy the mqtt manager object
     */
    ~MQTT_Manager();

private:

    /**
     * @brief Re-connection failure
     * 
     * @param tok result of async operation
     */
    void on_failure(const mqtt::token& tok) override;
    

    /**
     * @brief Action success
     * 
     * @param tok result of async operation
     */
    void on_success(const mqtt::token& tok) override {};

    /**
     * @brief (Re)connection success
     * 
     * @param cause message associated with this callback
     */
    void connected(const std::string& cause) override;

    /**
     * @brief Called when the connection is lost.
     * 
     * @param cause 
     */
    void connection_lost(const std::string& cause) override;

    /**
     * @brief Called when the message arrives.
     * 
     * @param msg
     */
    void message_arrived(mqtt::const_message_ptr msg) override;

    /**
     * @brief Split string {s} by {delimiter} and store result in {tokens}.
     * 
     * @param s 
     * @param delimiter 
     * @param tokens 
     */
    void split(const std::string& s, char delimiter, std::vector<std::string>& tokens);

    /**
     * @brief Write logs in the log file.
     * 
     * Write string to file for log informations.
     * Log message is defined by template:
     *  time -> event
     *  time is in the format: {hour}:{min}:{second}.
     * @param logMessage 
     */
    void recordLog(const std::string& logMessage);

};

#endif // MQTT_MANAGER_HPP