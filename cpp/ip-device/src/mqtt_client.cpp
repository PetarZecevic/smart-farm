#include "mqtt_client.hpp"
#include <iostream>
#include <ctime>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// Public
MQTT_Client::MQTT_Client(const IPInfo& info, std::string logFileName):
    ipstack_(),
    client_(ipstack_),
    ipinfo_(info),
    userid_(""),
    gatewayid_(""),
    reportAllowed_(false)
{
    updateFunction_ = NULL;
    try
    {
        logFile_.open("log/" + logFileName, std::ios::openmode::_S_out);
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    } 
}

void MQTT_Client::setLog(std::string ssdpLog)
{
    // Parse user Id.
    int pos = ssdpLog.find_first_of('/', 0);
    userid_ += ssdpLog.substr(0, pos);

    // Parse gateway Id.
    int begin,end;
    begin = ssdpLog.find("gateway/");
    begin += 8;
    end  = ssdpLog.find("/log");
    gatewayid_ += ssdpLog.substr(begin, end-begin);
    
    // Create all topics for communication with MQTT broker.
	std::string iplog = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/log";
    std::string gatewaylog = userid_ + "/gateway/" + gatewayid_ + "/log";
    std::string report = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/report";
    std::string update = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/update";
    std::string get = userid_ + "/" + ipinfo_.getByKey("group") + "/" + ipinfo_.getByKey("id") + "/get";
    
    // Insert topics into map.
    topics_.insert(std::make_pair("iplog", iplog));
    topics_.insert(std::make_pair("gatewaylog", gatewaylog));
    topics_.insert(std::make_pair("report", report));
    topics_.insert(std::make_pair("update", update));
    topics_.insert(std::make_pair("get", get));    
}

bool MQTT_Client::connectToBroker(std::string brokerLocation, int port)
{
    int rc = ipstack_.connect(brokerLocation.c_str(), port);
	if (rc != MQTT::returnCode::SUCCESS)
	{
        return false;
	}
	else
	{
		printf("MQTT connecting\n");
    	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    	data.MQTTVersion = 3;
        std::string id = ipinfo_.getByKey("id");
    	data.clientID.cstring = (char*)id.c_str();
        // Set last will message to notify gateway, when device disconnect.
        data.will.message.cstring = (char*) "OFFLINE";
        data.will.qos = MQTT::QOS1;
        data.will.retained = 0;
        data.will.topicName.cstring = (char*)topics_["report"].c_str();
        data.willFlag = 1;

        rc = client_.connect(data);
		if (rc != MQTT::returnCode::SUCCESS)
		{
	    	return false;
		}
    }	
    return true;
}

void MQTT_Client::report(ReportFunction* repFunc)
{
    while(true)
    {
        rapidjson::Document diffState;
        diffState.Parse("{}");
        if(repFunc->operator()(diffState, ipinfo_.getStateDOM()))
        {
            // Send changed state.
            if(!sendReport(diffState))
                break;
        }
        //TODO: Set number of seconds to be field of MQTTClient class.
        waitFor(2000); // reporting on every 2 seconds.
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

bool MQTT_Client::sendReport(rapidjson::Document& state)
{
    bool result = true;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    state.Accept(writer);

    int rc = client_.publish(topics_["report"].c_str(), (void*)buffer.GetString(), buffer.GetSize(), MQTT::QOS1);
    if(rc != MQTT::returnCode::SUCCESS)
        result = false;
    return result;
}

void MQTT_Client::logCallback(MQTT::MessageData& mdata)
{
    MQTT::Message &message = mdata.message;
    std::string response((const char*)message.payload, message.payloadlen);
    std::string logM("Response: ");
    logM += response;
    recordLog(logM);
    logM.clear();
    if(response == "OK")
    {
        logM = "Log success!\n";
        recordLog(logM);
        // Subsribe to previously defined topics.
        client_.subscribe(topics_["update"].c_str(), MQTT::QOS1, message_handler(this, &MQTT_Client::updateCallback));
        client_.subscribe(topics_["get"].c_str(), MQTT::QOS1, message_handler(this, &MQTT_Client::getCallback));
        // Publish permission for reporting state parameters.
        reportAllowed_ = true;
    }  
    else
    {
        logM = "Log failed!\n";
        recordLog(logM);
    }
}

void MQTT_Client::updateCallback(MQTT::MessageData& mdata)
{
    MQTT::Message& message = mdata.message;
    std::string update((const char*)message.payload, message.payloadlen);
    std::string logM("Parameters to update:\n");
    logM += update;
    recordLog(logM);
    logM.clear();

    rapidjson::Document newState;
    newState.Parse(update.c_str());
    if(!newState.HasParseError())
    {
        if(ipinfo_.mergeState(newState))
        {
            // Call update function callback.
            if(updateFunction_ != NULL)
            {
                updateFunction_->operator()(ipinfo_.getStateDOM(), newState);
            }
        }
        else
            // Failed to merge state.
            return;
    }
}

void MQTT_Client::getCallback(MQTT::MessageData& mdata)
{
    MQTT::Message& message = mdata.message;
    std::string request((const char*)message.payload, message.payloadlen);
    std::string logM("Parameters to return:\n");
    logM += request;
    recordLog(logM);
}

void MQTT_Client::waitFor(int milliseconds)
{
    client_.yield(milliseconds);
}

bool MQTT_Client::subscribe()
{
    int rc = client_.subscribe(topics_["iplog"].c_str(), MQTT::QOS2, message_handler(this, &MQTT_Client::logCallback));
       
	if (rc != MQTT::returnCode::SUCCESS)
		return false;
    else
        return true;
}

bool MQTT_Client::sendInfo()
{
    std::string builder = "";
    builder += ipinfo_.getDescriptionString();
    //std::cout << builder << std::endl;
    //std::cout << "length: " << builder.length() << std::endl;
    int rc = client_.publish(topics_["gatewaylog"].c_str(), (void*)builder.c_str(), builder.length(), MQTT::QOS2);

    if(rc != MQTT::returnCode::SUCCESS)
        return false;
    else
        return true;
}

MQTT_Client::~MQTT_Client(){
    if(logFile_.is_open()){
        logFile_.close();
    }
}
