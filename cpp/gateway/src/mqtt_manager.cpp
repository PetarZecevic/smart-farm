#include "mqtt_manager.hpp"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <ctime>

MQTT_Manager::MQTT_Manager(std::string userId, std::string gatewayId, std::string brokerAddress):
    user_id_(userId),
    gateway_id_(gatewayId),
    broker_addr_(brokerAddress),
    mqtt_client_(broker_addr_, gateway_id_)
{
    mqtt_client_.set_callback(*this);
    manager_topic_prefix_ = user_id_ + "/gateway/" + gateway_id_;
    try{
        logFile_.open("log/mqtt-log.txt", std::ios::openmode::_S_out);
    }catch(std::exception& e){
        std::cout << "Unable to open mqtt log file." << std::endl;
    }
}

bool MQTT_Manager::connectToBroker(mqtt::connect_options coptions)
{
    try
    {
        mqtt::token_ptr conntok = mqtt_client_.connect(coptions);
        conntok->wait_for(500);
    }catch(const mqtt::exception& exc)
    {
        std::string excp = exc.what();
        recordLog(excp);
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
            // TODO: Add factory pattern for handlers and pass it to the function.
            std::unique_ptr<ITopicHandler> devRegHandler(new DeviceRegisterHandler);
            subscribeWithHandler(managerTopicPrefix_ + "/log", 2, devRegHandler);
            std::unique_ptr<ITopicHandler> commandHandler(new CommandHandler); 
            subscribeWithHandler(managerTopicPrefix_ + "/command", 1, commandHandler); 
        }
        catch(const mqtt::exception& exc)
        {
            std::string excp = exc.what();
            recordLog(excp);
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

// TODO: Move this function as the part of the Device class.
void MQTT_Manager::mergeDeviceState(std::string id, const rapidjson::Document& state)
{
    static std::string kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    auto it = devices_.find(id);
    if(it != devices_.end())
    {
        Device& device = devices_.at(id);
        rapidjson::Document::AllocatorType& allocator = device.state.GetAllocator();
        // Go through services.
        for (rapidjson::Value::ConstMemberIterator itr = state.MemberBegin(); 
            itr != state.MemberEnd(); 
            ++itr)
        {
            if(device.state.HasMember(itr->name.GetString()))
            {
                const rapidjson::Value& params = state[itr->name.GetString()];
                if(params.IsObject())
                {
                    // Go through params for each service.
                    for(rapidjson::Value::ConstMemberIterator pit = params.MemberBegin(); 
                        pit != params.MemberEnd(); 
                        ++pit)
                    {
                        // Update params values.
                        rapidjson::Value& v = device.state[itr->name.GetString()][pit->name.GetString()];
                        if(kTypeNames[pit->value.GetType()] == "String")
                        {
                            v.SetString(pit->value.GetString(), allocator);
                        }
                        else if(kTypeNames[pit->value.GetType()] == "Number")
                        {
                            // Support int and double.
                            if(pit->value.IsInt())
                            {
                                v.SetInt(pit->value.GetInt());
                            }
                            else if(pit->value.IsDouble())
                            {
                                v.SetDouble(pit->value.GetDouble());
                            }
                        }
                    }
                }
                else
                    break;
            }
        }
    }
}

std::string MQTT_Manager::getAllDevicesInfo()
{
    std::string info = "";
    info += "{"; // begin object.
    info += "\"info\":["; // start array.
    
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

std::string MQTT_Manager::getAllDevicesState()
{
    std::string state = "";
    state += "{"; // begin object.
    state += "\"state\":["; // start array.
    
    auto it = devices_.begin();
    if(it != devices_.end())
    {
        state += it->second.getState();
        ++it;
        for(;it != devices_.end(); ++it)
        {
            state += ',';
            state += it->second.getState();
        }
    }

    state += "]"; // end array.
    state += "}"; // end object.

    return state;
}


std::string MQTT_Manager::getDeviceInfo(std::string id)
{
    std::string info = "";
    info += "{"; // begin object.
    info += "\"info\":";
    info += "[";
    auto it = devices_.find(id);
    if(it != devices_.end())
    {
        info += it->second.getInfo();
    }
    else
    {
        info += "{}";
    }
    
    info += "]"; // end array.
    info += "}"; // end object.
    return info;
}

std::string MQTT_Manager::getDeviceState(std::string id)
{
    std::string state = "";
    state += "{"; // begin object.
    state += "\"state\":";
    state += "["; // start array.
    auto it = devices_.find(id);
    if(it != devices_.end())
    {
        state += it->second.getState();
    }
    else
    {
        state += "{}";
    }
    
    state += "]"; // end array.
    state += "}"; // end object.
    return state;
}

void MQTT_Manager::subsrcibeWithHandler(std::string topic, int QoS, std::unique_ptr<ITopicHandler>& handlerPtr)
{
    mqtt_client_.subscribe(topic, QoS);
    topic_handlers_.insert(std::make_pair(topic, std::move(handlerPtr));
}

void MQTT_Manager::recordLog(const std::string& logMessage)
{
    if(logFile_.is_open())
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        logFile_ << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << " -> ";
        logFile_ << logMessage << std::endl;
    }
}

MQTT_Manager::~MQTT_Manager()
{
    if(logFile_.is_open())
    {
        std::string bye = "MQTT Manager closed";
        recordLog(bye); 
        logFile_.close();
    }

    // Release each registered handler.

}


void MQTT_Manager::on_failure(const mqtt::token& tok){
    std::string logM = "Connection attempt failed";
    recordLog(logM);
}

void MQTT_Manager::connected(const std::string& cause){
	std::string logM = "Connection success";
    recordLog(logM);
}

void MQTT_Manager::connection_lost(const std::string& cause) 
{
    std::string logM1 = "Connection lost";
    recordLog(logM1);
    if (!cause.empty())
    {
        std::string logM2 = "cause: " + cause;
        recordLog(logM2); 
    }
}

void MQTT_Manager::message_arrived(mqtt::const_message_ptr msg)
{
    std::string topic = msg->get_topic();
    std::string message = msg->to_string();

    auto handler = topic_handlers_.find(topic);
    if(handler != topic_handlers_.end())
    {
        handler->handle(message);
    }
    else
    {  
        std::string excp = "Handler for the topic not found";
        recordLog(excp);
    }
}


// Handlers
void DeviceRegisterHandler::handle(std::string message)
{
    rapidjson::Document deviceInfo;
    deviceInfo.Parse(content.c_str());
    if(!deviceInfo.HasParseError())
    {
        std::string deviceId = deviceInfo["id"].GetString();
        std::string deviceGroup = deviceInfo["group"].GetString();
        std::string iplog = getUserId() + "/" + deviceGroup + "/" + deviceId + "/log";
        std::string ipreport = getUserId() + "/" + deviceGroup + "/" + deviceId + "/report";
        try
        {
            std::unique_ptr<ITopicHandler> devReportHandlerPtr(new DeviceReportHandler(deviceId))
            subscribeWithHandler(ipreport, 1, devReportHandlerPtr);
            mqtt::message_ptr pubm = mqtt::make_message(iplog, "OK");
            pubm->set_qos(1);
            mqtt_client_.publish(pubm);
        }
        catch(mqtt::exception& exc)
        {
            std::string excp = exc.what();
            recordLog(excp);
            return;
        }

        rapidjson::Document state;
        std::string logM("");
        if(Device::setStateFromDescription(state, deviceInfo))
        {
            // Successfully configured state from description.
            // Register device into internal database.
            registerDevice(deviceId, deviceInfo, state);
            logM += "Device " + deviceId + " connected."; 
            recordLog(logM);
        }
        else
        {
            // Error in description.
            logM += "Device " + deviceId + " state configuring failed.";
            recordLog(logM);
        }
    }
}

void CommandHandler::handle(std::string message)
{
    std::string logM("");
    logM += "Command from user:\n";
    logM += message;
    recordLog(logM);
    // Parse user command.
    rapidjson::Document command;
    command.Parse(message.c_str());
    std::string type = command["command_type"].GetString();

    if(type == "GET")
    {
        std::string json = command["json"].GetString();
        std::string device = command["device"].GetString();
        std::string response = "";
        if(json == "info")
        {
            topic += "/response/info"; 
            if(device == "*")
                response = getAllDevicesInfo();
            else
                response = getDeviceInfo(device);
        }
        else if(json == "state")
        {
            topic += "/response/state";
            if(device == "*")
                response = getAllDevicesState();
            else
                response = getDeviceState(device);
        }

        if(!message.empty())
        {
            try
            {
                mqtt::message_ptr pubm = mqtt::make_message(topic, response.c_str());
                pubm->set_qos(1);
                mqtt_client_.publish(pubm);
            }
            catch(const mqtt::exception& e)
            {
                std::string excp = e.what();
                recordLog(excp);
            }
        }
    }
    else if(type == "SET")
    {
        
        // Change state of one of the devices.
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        
        writer.StartObject();
        writer.Key(command["service"].GetString());
        writer.StartObject();
        writer.Key(command["parameter"].GetString());
        writer.String(command["value"].GetString());
        writer.EndObject();
        writer.EndObject();

        // Send command to the device.
        std::string deviceTopic = getUserId() + "/" + 
                                    command["group"].GetString() + "/" +
                                    command["device"].GetString() + "/update"; 
        try
        {
            mqtt::message_ptr pubm = mqtt::make_message(deviceTopic, s.GetString());
            pubm->set_qos(1);
            mqtt_client_.publish(pubm);
        }
        catch(const mqtt::exception& e)
        {
            std::string excp = e.what();
            recordLog(excp);
        }

        rapidjson::Document state;
        state.Parse(s.GetString());
        mergeDeviceState(command["device"].GetString(), state);   
    }
}

DeviceReportHandler::DeviceReportHandler(std::string deviceId) :
    device_id_(deviceId)
{}

void DeviceReportHandler::handle(std::string message)
{
    if(message == "OFFLINE")
    {
        // Device disconnected, delete it from the devices table.
        unregisterDevice(device_id_);
        std::string logM("");
        logM += "Device " + device_id_ + " disconnected."; 
        recordLog(logM);
    }
    else
    {
        // Merge change in the parameters.
        rapidjson::Document state;
        state.Parse(message.c_str());
        if(!state.HasParseError())
        {
            mergeDeviceState(device_id_, state);
        }
    }
}

void MQTT_Manager::split(const std::string& s, char delimiter, std::vector<std::string>& tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}