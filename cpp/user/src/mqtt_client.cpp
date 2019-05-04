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
    topics_["response"] = topics_["command"] + "/response";
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
            mqttClient_.subscribe(topics_["response"], 1);
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
    for(size_t i = 0; i < devices_.size(); i++)
    {
        if(devices_[i].IsObject())
        {
            devs[devices_[i]["id"].GetString()].CopyFrom(devices_[i], devices_[i].GetAllocator());
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
    /*
        Topics to react:
            - command/response, command/status.
    */
    if(topic == parent_.getTopic("response"))
    {
        // Go through json and store all devices into map.
        rapidjson::Document devs;
        devs.Parse(content.c_str());
        if(!devs.HasParseError())
        {
            rapidjson::Value arr = devs["devices"].GetArray();
            parent_.devices_.clear();
            if(!arr.Empty())
            {
                parent_.devices_.resize(arr.Size()); 
                for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
                {
                    rapidjson::Value device = arr[i].GetObject();
                    parent_.devices_[i].CopyFrom(device, parent_.devices_[i].GetAllocator());       
                }
            }
        }
    }
}