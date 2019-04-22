#include "mqtt_manager.hpp"
#include <rapidjson/document.h>

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
    
    std::string topic = msg->get_topic();
    std::string content = msg->to_string();
    if(topic == managerLog_)
    {
        rapidjson::Document deviceInfo;
        deviceInfo.Parse(content.c_str());
        if(!deviceInfo.HasParseError())
        {
            std::string iplog = manager_.getUserId() + "/" + deviceInfo["group"].GetString() + "/" + deviceInfo["id"].GetString() + "/log";
            std::string ipreport = manager_.getUserId() + "/" + deviceInfo["group"].GetString() + "/" + deviceInfo["id"].GetString() + "/report";
            try
            {
                cli_.subscribe(ipreport, 1);
                mqtt::message_ptr pubm = mqtt::make_message(iplog, "OK");
                pubm->set_qos(1);
                cli_.publish(pubm);
            }
            catch(mqtt::exception& exc)
            {
                std::cout << exc.what() << std::endl;
            } 
        }
    }
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
