#include "mqtt_server.hpp"

// Callback.
// Re-connection failure
void MQTT_Server::callback::on_failure(const mqtt::token& tok){
    std::cout << "Connection attempt failed" << std::endl;
}

void MQTT_Server::callback::connected(const std::string& cause){
	std::cout << "\nConnection success" << std::endl;
}

void MQTT_Server::callback::connection_lost(const std::string& cause) 
{
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty())
        std::cout << "\tcause: " << cause << std::endl;
}

void MQTT_Server::callback::message_arrived(mqtt::const_message_ptr msg)
{
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
        std::string content = msg->to_string();
        std::string iptopic = parseIpTopic(content);
        mqtt::message_ptr m = mqtt::make_message(iptopic, "Hello from MQTT Server!");
        cli_.publish(m);
}


std::string MQTT_Server::callback::parseIpTopic(std::string message)
{
    return manager_.getUserId() + "/" + message + "/log";
}

// Server.
MQTT_Server::MQTT_Server(std::string userId, std::string gatewayId, std::string brokerAddress):
    user_id(userId),
    gateway_id(gatewayId),
    broker_addr(brokerAddress),
    server(broker_addr, user_id),
    cb(server, *this)
{
    server.set_callback(cb);
}

bool MQTT_Server::connectToBroker(mqtt::connect_options coptions)
{
    try
    {
        mqtt::token_ptr conntok = server.connect(coptions);
        conntok->wait_for(500);
    }catch(const mqtt::exception& exc)
    {
        std::cerr << exc.what() << std::endl;
        return false;
    }
    return true;
}

bool MQTT_Server::start()
{
    if(server.is_connected())
    {
        try
        {
            std::string logTopic = user_id + "/gateway/" + gateway_id + "/log";
            server.subscribe(logTopic, 2);
        }
        catch(const mqtt::exception& exc)
        {
            std::cerr << exc.what() << std::endl;
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}