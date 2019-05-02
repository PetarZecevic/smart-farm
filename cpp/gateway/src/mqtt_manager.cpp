#include "mqtt_manager.hpp"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>

// Manager.
MQTT_Manager::MQTT_Manager(std::string userId, std::string gatewayId, std::string brokerAddress):
    user_id_(userId),
    gateway_id_(gatewayId),
    broker_addr_(brokerAddress),
    mqtt_client_(broker_addr_, gateway_id_),
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
            std::string commandTopic = user_id_ + "/gateway/" + gateway_id_ + "/command";
            mqtt_client_.subscribe(logTopic, 2);
            mqtt_client_.subscribe(commandTopic, 1);
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

void MQTT_Manager::registerDevice(std::string id, rapidjson::Document& info, rapidjson::Document& state)
{
    devices_.insert(std::make_pair(id, Device(info, state)));
}

void MQTT_Manager::unregisterDevice(std::string id)
{
    devices_.erase(id);
}

void MQTT_Manager::mergeDeviceState(std::string id, const rapidjson::Document& state)
{
    static std::string kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    auto it = devices_.find(id);
    if(it != devices_.end())
    {
        Device& d = devices_.at(id);
        rapidjson::Document::AllocatorType& allocator = d.state.GetAllocator();
        for (rapidjson::Value::ConstMemberIterator itr = state.MemberBegin(); itr != state.MemberEnd(); ++itr)
        {
            if(d.state.HasMember(itr->name.GetString()))
            {
                rapidjson::Value& v = d.state[itr->name.GetString()];
                if(kTypeNames[itr->value.GetType()] == "String")
                {
                    v.SetString(itr->value.GetString(), allocator);
                }
                else if(kTypeNames[itr->value.GetType()] == "Number")
                {
                    // Support int and double.
                    if(itr->value.IsInt())
                    {
                        v.SetInt(itr->value.GetInt());
                    }
                    else if(itr->value.IsDouble())
                    {
                        v.SetDouble(itr->value.GetDouble());
                    }
                }
            }
        }
    }
}

std::string MQTT_Manager::getDevicesInfo()
{
    std::string info = "";
    info += "{"; // begin object.
    info += "\"devices\":["; // start array.
    
    auto it = devices_.begin();
    if(it != devices_.end())
    {
        info += it->second.getInfo();
        ++it;
        for(;it != devices_.end(); ++it)
        {
            info += ',';
            info += it->second.getInfo();
        }
    }

    info += "]"; // end array.
    info += "}"; // end object.

    return info;
}

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
            std::string deviceId = deviceInfo["id"].GetString();
            std::string deviceGroup = deviceInfo["group"].GetString();
            std::string iplog = manager_.getUserId() + "/" + deviceGroup + "/" + deviceId + "/log";
            std::string ipreport = manager_.getUserId() + "/" + deviceGroup + "/" + deviceId + "/report";
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
                return;
            }
            // Register device into database.
            // Parse devices state.
            rapidjson::Document state;
            state.Parse("{}");
            const rapidjson::Value& params = deviceInfo["parameters"];
            if(params.IsArray())
            {
                rapidjson::StringBuffer s;
                rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                writer.StartObject();
                for(rapidjson::SizeType i = 0; i < params.Size(); i++)
                {
                    writer.Key(params[i].GetString());
                    writer.Null();
                }
                writer.EndObject();
                state.Parse(s.GetString());
                if(state.HasParseError())
                   state.Parse("{}"); 
            }
            manager_.registerDevice(deviceId, deviceInfo, state); 
        }
    }
    else
    {
        // Parse topic to find out method and device's id.
        std::vector<std::string> tokens;
        split(topic, '/', tokens);
        size_t lastToken = tokens.size()-1;
        std::string method = tokens[lastToken];
        
        if(method == "report")
        {
            std::string deviceId = tokens[lastToken-1];
            if(content == "OFFLINE")
            {
                // Device disconnected, delete it from devices list.
                manager_.unregisterDevice(deviceId);
            }
            else
            {
                // Merge change in parameters.
                rapidjson::Document state;
                state.Parse(content.c_str());
                if(!state.HasParseError())
                {
                    manager_.mergeDeviceState(deviceId, state);
                }
            }
        }
        else if(method == "command")
        {
            // Parse user command.
            rapidjson::Document command;
            command.Parse(content.c_str());
            std::string type = command["command_type"].GetString();
            if(type == "ALL")
            {
                try
                {
                    // Send information about all devices.
                    topic += "/response";
                    mqtt::message_ptr pubm = mqtt::make_message(topic, manager_.getDevicesInfo().c_str());
                    pubm->set_qos(1);
                    cli_.publish(pubm);
                }
                catch(const mqtt::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            else if(type == "SET")
            {
                
                // Change state of one of the devices.
                rapidjson::StringBuffer s;
                rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                
                writer.StartObject();
                writer.Key(command["parameter"].GetString());
                writer.String(command["value"].GetString());
                writer.EndObject();

                // Send command to device.
                std::string deviceTopic = manager_.getUserId() + "/" + 
                                          command["group"].GetString() + "/" +
                                          command["device"].GetString() + "/update"; 
                try
                {
                    mqtt::message_ptr pubm = mqtt::make_message(deviceTopic, s.GetString());
                    pubm->set_qos(1);
                    cli_.publish(pubm);
                }
                catch(const mqtt::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }

                rapidjson::Document state;
                state.Parse(s.GetString());
                manager_.mergeDeviceState(command["device"].GetString(), state);   
            }
        }
        else
        {
            // error - bad topic.
            std::cout << "Error in topic" << std::endl;   
        }
    }
}

void MQTT_Manager::callback::split(const std::string& s, char delimiter, std::vector<std::string>& tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}