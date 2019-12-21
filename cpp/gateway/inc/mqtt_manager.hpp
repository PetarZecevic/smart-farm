#ifndef MQTT_MANAGER_HPP
#define MQTT_MANAGER_HPP

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <functional>
#include <memory>
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
    /* Table that represents IOT devices connected to the gateway */
    std::unordered_map<std::string, Device> devices_;
    class ITopicHandler; // Forward declaration of the interface.
    /* Table that maps gateway topics to handler called when message arrives at the specific topic */
    std::unordered_map<std::string, std::unique_ptr<ITopicHandler>> topic_handlers_;
    /* String prefix for gateway topics used by the other devices to communicate with the gateway */
    std::string manager_topic_prefix_;
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
    
    friend void ITopicHandler
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
     * 
     * Wrap around the default subscribe function of the mqtt client which will 
     * allow us to set the specific handler for the topic, that manager subscribes to.
     * 
     * @param topic subsription topic 
     * @param QoS quality of service, number between 0-2
     * @param handlerPtr pointer to handler which will activated when message arrives on the topic
     */
    void subsrcibeWithHandler(std::string topic, int QoS, std::unique_ptr<ITopicHandler>& handlerPtr);

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

    /**
     * @brief Used to handle message received on specific topic.
     */
    class ITopicHandler
    {
    public:
        virtual void handle(std::string message) = 0;
    };

    /**
     * @brief Used to handle registration of the new device to gateway devices table.
     * 
     */
    class DeviceRegisterHandler : public ITopicHandler
    {
    public:
        void handle(std::string message) override;
    };

    /**
     * @brief Used to handle commands sent by gateway user.
     * 
     */
    class CommandHandler : public ITopicHandler
    {
    public:
        void handle(std::string message) override;
    }

    /**
     * @brief Used to handle reports coming from registered devices.
     * 
     */
    class DeviceReportHandler : public ITopicHandler
    {
        /* Unique identification of the device used for the operations on the gateway device table. */
        std::string device_id_;
    public:
        /**
         * @brief Construct a new Device Report Handler object for device with given id.
         * 
         * @param deviceId device unique identification
         */
        DeviceReportHandler(std::string deviceId);
        void handle(std::string message) override;
    }
};

#endif // MQTT_MANAGER_HPP