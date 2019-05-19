#include "mqtt_client.hpp"
#include <ctime>

MQTT_Client::MQTT_Client(std::string userId, std::string gatewayId, std::string brokerAddress):
    userId_(userId),
    gatewayId_(gatewayId),
    brokerAddress_(brokerAddress),
    mqttClient_(brokerAddress_, userId_),
    cb_(*this)
{
    mqttClient_.set_callback(cb_);
    topics_["command"] = userId_ + "/gateway/" + gatewayId_ + "/command";
    topics_["status"] = topics_["command"] + "/status";
    topics_["response-info"] = topics_["command"] + "/response/info";
    topics_["response-state"] = topics_["command"] + "/response/state";
    try
    {
        logFile_.open("log/mqtt-log.txt", std::ios::openmode::_S_out);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

bool MQTT_Client::connectToBroker(mqtt::connect_options coptions)
{
    try
    {
        mqtt::token_ptr conntok = mqttClient_.connect(coptions);
        conntok->wait_for(500);
    }catch(const mqtt::exception& exc)
    {
        std::string logM = exc.what();
        recordLog(logM);
        return false;
    }
    return true;
}

bool MQTT_Client::start()
{
    if(mqttClient_.is_connected())
    {
        try
        {
            mqttClient_.subscribe(topics_["status"], 1);
            mqttClient_.subscribe(topics_["response-info"], 1);
            mqttClient_.subscribe(topics_["response-state"], 1);
        }
        catch(const mqtt::exception& exc)
        {
            std::string logM = exc.what();
            recordLog(logM);
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool MQTT_Client::sendCommand(rapidjson::Document& command)
{
    if(mqttClient_.is_connected())
    {
        try
        {
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            command.Accept(writer);
            mqtt::message_ptr pubm = mqtt::make_message(topics_["command"], s.GetString());
            pubm->set_qos(1);
            mqttClient_.publish(pubm);
        }
        catch(const mqtt::exception& exc)
        {
            std::string logM = exc.what();
            recordLog(logM);
            return false;
        }
        return true;
    }
    else
        return false;
}

void MQTT_Client::getAllDevicesInfo(std::unordered_map<std::string, rapidjson::Document>& devs)
{
    devs.clear();
    for(size_t i = 0; i < devicesInfo_.size(); i++)
    {
        if(devicesInfo_[i].IsObject())
        {
            devs[devicesInfo_[i]["id"].GetString()].CopyFrom(devicesInfo_[i], devicesInfo_[i].GetAllocator());
        }
    }
}

void MQTT_Client::getAllDevicesState(std::unordered_map<std::string, rapidjson::Document>& devs)
{
    devs.clear();
    for(size_t i = 0; i < devicesState_.size(); i++)
    {
        if(devicesState_[i].IsObject())
        {
            devs[devicesState_[i]["id"].GetString()].CopyFrom(devicesState_[i], devicesState_[i].GetAllocator());
        }
    }
}

void MQTT_Client::recordLog(const std::string& logMessage)
{
    if(logFile_.is_open())
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        logFile_ << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << " -> ";
        logFile_ << logMessage << std::endl;
    }
}

MQTT_Client::~MQTT_Client()
{
    if(logFile_.is_open())
    {
        logFile_.close();
    }
}

// Private

// Callback

void MQTT_Client::callback::on_failure(const mqtt::token& tok)
{
    std::string logM("Connection attempt failed");
    parent_.recordLog(logM);
}

void MQTT_Client::callback::connected(const std::string& cause)
{
    std::string logM("Connected to MQTT broker");
    parent_.recordLog(logM); 
}

void MQTT_Client::callback::connection_lost(const std::string& cause)
{
    std::string logM("\nConnection lost");
    if (!cause.empty())
        logM += "\tcause: " + cause;
    parent_.recordLog(logM);
}

void MQTT_Client::callback::message_arrived(mqtt::const_message_ptr msg)
{
    std::string topic = msg->get_topic();
    std::string content = msg->get_payload_str();
    std::string logM("Message: \n");
    logM += content;
    parent_.recordLog(logM);
    
    if(topic == parent_.getTopic("response-info"))
    {
        rapidjson::Document devs;
        devs.Parse(content.c_str());
        if(!devs.HasParseError())
        {
            rapidjson::Value arr = devs["info"].GetArray();
            parent_.devicesInfo_.clear();
            if(!arr.Empty())
            {
                parent_.devicesInfo_.resize(arr.Size()); 
                for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
                {
                    rapidjson::Value device = arr[i].GetObject();
                    parent_.devicesInfo_[i].CopyFrom(device, parent_.devicesInfo_[i].GetAllocator());       
                }
            }
        }
    }
    else if(topic == parent_.getTopic("response-state"))
    {
        rapidjson::Document devs;
        devs.Parse(content.c_str());
        if(!devs.HasParseError())
        {
            rapidjson::Value arr = devs["state"].GetArray();
            parent_.devicesState_.clear();
            if(!arr.Empty())
            {
                parent_.devicesState_.resize(arr.Size()); 
                for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
                {
                    rapidjson::Value device = arr[i].GetObject();
                    parent_.devicesState_[i].CopyFrom(device, parent_.devicesState_[i].GetAllocator());       
                }
            }
        }
    }
}   