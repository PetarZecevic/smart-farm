#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <unordered_map>
#include <mqtt/async_client.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

class MQTT_Client
{
public:
    MQTT_Client(std::string userId, std::string gatewayId, std::string brokerAddress);
    bool connectToBroker(mqtt::connect_options coptions);
    bool start();
    bool sendCommand(rapidjson::Document& command);
    void getAllDevicesInfo(std::unordered_map<std::string, rapidjson::Document>& devs);
    mqtt::async_client& getClient() {return mqttClient_;};
    std::vector<rapidjson::Document>& getDevices() { return devices_;};
    std::string getTopic(const char* tag) { return topics_[tag];};
private:
    std::string userId_;
    std::string gatewayId_;
    std::string brokerAddress_;
    mqtt::async_client mqttClient_;
    class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener
    {
        // The MQTT client that calls this callback.
        MQTT_Client& parent_;
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
    public:
        callback(MQTT_Client& parent): 
            parent_(parent)
        {}
    };
    callback cb_;
    std::unordered_map<const char*, std::string> topics_;
    std::vector<rapidjson::Document> devices_;
};

#endif // MQTT_CLIENT_HPP
