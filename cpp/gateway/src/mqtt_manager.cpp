#include "mqtt_manager.hpp"

// Callback.
// Re-connection failure
void MQTT_Manager::callback::on_failure(const mqtt::token& tok){
    std::cout << "Connection attempt failed" << std::endl;
}

void MQTT_Manager::callback::connected(const std::string& cause){
	std::cout << "\nConnection success" << std::endl;
}

void MQTT_Manager::callback::connection_lost(const std::string& cause) 
{
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty())
        std::cout << "\tcause: " << cause << std::endl;
}

void MQTT_Manager::callback::message_arrived(mqtt::const_message_ptr msg)
{
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
        //std::string content = msg->to_string();
        //std::string iptopic = parseIpTopic(content);
        //mqtt::message_ptr m = mqtt::make_message(iptopic, "Hello from MQTT Server!");
        //cli_.publish(m);
}


std::string MQTT_Manager::callback::parseIpTopic(std::string message)
{
    return manager_.getUserId() + "/" + message + "/log";
}

// Server.
MQTT_Manager::MQTT_Manager(std::string userId, std::string gatewayId, std::string brokerAddress):
    user_id_(userId),
    gateway_id_(gatewayId),
    broker_addr_(brokerAddress),
    mqtt_client_(broker_addr_, user_id_),
    cb_(mqtt_client_, *this)
{
    mqtt_client_.set_callback(cb_);
}

bool MQTT_Manager::connectToBroker(mqtt::connect_options coptions)
{
    try
    {
        mqtt::token_ptr conntok = mqtt_client_.connect(coptions);
        conntok->wait_for(500);
    }catch(const mqtt::exception& exc)
    {
        std::cerr << exc.what() << std::endl;
        return false;
    }
    return true;
}

bool MQTT_Manager::start()
{
    if(mqtt_client_.is_connected())
    {
        try
        {
            std::string logTopic = user_id_ + "/gateway/" + gateway_id_ + "/log";
            mqtt_client_.subscribe(logTopic, 2);
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