#ifndef MQTT_SERVER_HPP
#define MQTT_SERVER_HPP

#include <string>
#include <iostream>
#include <mqtt/async_client.h>


class MQTT_Server
{
public:
    MQTT_Server(std::string userId, std::string gatewayId, std::string brokerAddress);
    bool connectToBroker(mqtt::connect_options coptions);
    bool start();
    std::string getUserId() const {return user_id;};
private:
    std::string user_id;
    std::string gateway_id;
    std::string broker_addr;
    mqtt::async_client server;
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
        MQTT_Server& manager_;

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
        std::string parseIpTopic(std::string message);
        void delivery_complete(mqtt::delivery_token_ptr token) override {}

    public:
        callback(mqtt::async_client& cli, MQTT_Server& manager): 
            cli_(cli),
            manager_(manager)
        {}
    };
    callback cb;
};

#endif // MQTT_SERVER_HPP