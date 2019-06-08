#include "parser.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <list>

Parser::Parser(std::unordered_map<std::string, rapidjson::Document>& dInfo,
        std::unordered_map<std::string, rapidjson::Document>& dState,
        std::unordered_map<std::string, Parameter*>& pInfo):
        devicesInfo_(dInfo),
        devicesState_(dState),
        paramsInfo_(pInfo)
{
            // Configure command templates.
            templateSet_.Parse("{}");
            templateGet_.Parse("{}");
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            // Set.
            writer.StartObject();
            writer.Key("command_type"); writer.Null();
	        writer.Key("group"); writer.Null();
	        writer.Key("device"); writer.Null();
	        writer.Key("service"); writer.Null();
	        writer.Key("parameter"); writer.Null();
	        writer.Key("value"); writer.Null();
            writer.EndObject();
            templateSet_.Parse(s.GetString());
            writer.Reset(s);
            // Get.
            writer.StartObject();
            writer.Key("command_type"); writer.Null();
	        writer.Key("json"); writer.Null();
	        writer.Key("device"); writer.Null();
            writer.EndObject();
            templateGet_.Parse(s.GetString());
}

bool Parser::parseCommand(std::string command, rapidjson::Document& jsonCommand, std::string& errorMessage)
{
    bool success = false;
    std::list<std::string> tokens;
    split(command, ' ', tokens);
    if(!tokens.empty() && tokens.size() >= 1)
    {
        if(tokens.front() == "SET")
        {
            tokens.pop_front();
            success = methodSet(tokens, errorMessage);
        }
        else if(tokens.front() == "GET")
        {
            tokens.pop_front();
            success = methodGet(tokens, errorMessage);
        }
        else
            errorMessage = "Command type not recognized";
    }
    else
        errorMessage = "Wrong command format";
    
    return success;
}

bool Parser::methodSet(std::list<std::string>& tokens, std::string& errorMessage)
{
    bool success = false;
    if(tokens.size() == 2)
    {
        std::list<std::string> parameterTokens;
        split(tokens.front(), '.', parameterTokens);
        tokens.pop_front();

        if(parameterTokens.size() == 3)
        {
            std::string value = tokens.front();
            // device_id.service.param
            std::string id = parameterTokens.front();
            parameterTokens.pop_front();
            std::string service = parameterTokens.front();
            parameterTokens.pop_front();
            std::string param = parameterTokens.front();
            
            std::string key = id + "|" + service + "|" + param; // Key for cache.
            // Try to find device based on id from command.
            if(devicesInfo_.find(id) != devicesInfo_.end())
            {
                // Found device.
                if(devicesInfo_[id].HasMember(service.c_str()))
                {
                    if(devicesInfo_[id][service.c_str()].HasMember(param.c_str()))
                    {
                        // Found parameter.
                        templateSet_["group"].SetString(devicesInfo_[id]["group"].GetString(), templateSet_.GetAllocator());
                        templateSet_["device"].SetString(id.c_str(), templateSet_.GetAllocator());
                        templateSet_["service"].SetString(service.c_str(), templateSet_.GetAllocator());
                        templateSet_["parameter"].SetString(param.c_str(), templateSet_.GetAllocator());
                        // First check for cache.
                        if(paramsInfo_.find(key) != paramsInfo_.end())
                        {
                            // Hit success.
                            // Check if value for parameter is valid.
                            if(paramsInfo_[key]->isValueAllowed(value))
                            {
                                success = true;
                                templateSet_["value"].SetString(value.c_str(), templateSet_.GetAllocator());
                            }
                            else
                               errorMessage = "ERROR: Bad Parameter value.";
                        }
                        else
                        {
                            // Hit failed, parse parameter info, and store it in cache.
                            bool writeAllowed;
                            std::string parameterInfo(devicesInfo_[id][service.c_str()][param.c_str()].GetString());
                            std::vector<std::string> tok;
                            split(parameterInfo, '|', tok);
                            std::string values = tok[0];
                            // Check if parameter is read only.
                            if(tok.size() == 1)
                            {
                                writeAllowed = true;
                            }
                            else if(tok[1] == "r")
                            {
                                writeAllowed = false;
                            }
                            tok.clear();
                            // Check for list based parameter.
                            split(values, '/', tok);
                            if(tok.size() > 1)
                            {
                                paramsInfo_[key] = new ListParameter(tok, writeAllowed);
                                if(!writeAllowed)
                                {
                                    errorMessage = "ERROR: Read-only parameter."; 
                                }
                                else if(paramsInfo_[key]->isValueAllowed(value))
                                {
                                    success = true;
                                    templateSet_["value"].SetString(value.c_str(), templateSet_.GetAllocator());
                                }
                                else
                                    errorMessage = "ERROR: Parameter value " + value + " not in list " + values;
                            }
                            // Ranged based parameter.
                            else
                            {   
                                split(values, '~', tok);
                                int minimum = std::stoi(tok[0]);
                                int maximum = std::stoi(tok[1]);
                                paramsInfo_[key] = new RangeParameter(minimum, maximum, writeAllowed);
                                if(!writeAllowed)
                                {
                                    errorMessage = "ERROR: Read-only parameter."; 
                                }
                                else if(paramsInfo_[key]->isValueAllowed(value))
                                {
                                    success = true;
                                    templateSet_["value"].SetString(value.c_str(), templateSet_.GetAllocator());
                                }
                                else
                                    errorMessage = "ERROR: Parameter value " + value + " not in range " + values;
                            }
                        }
                    }
                    else
                        errorMessage = "ERROR: Parameter " + param + " not found.";						
                }
                else
                    errorMessage = "ERROR: Service " + service + " not found.";
            }
            else
                errorMessage = "ERROR: Device " + id + " not found.";					
        }
        else
            errorMessage = "ERROR: Expected deviceId.paramName expression.";
    }
    else
        errorMessage = "Not enough params for method";

    return success;
}

bool Parser::methodGet(std::list<std::string>& tokens, std::string& errorMessage)
{
    bool success = false;
    if(tokens.size() == 1)
    {
        std::vector<std::string> parameterTokens;
        split(tokens.front(), '.', parameterTokens);
        if(parameterTokens.size() == 2)
        {
            std::string id = parameterTokens[0];
            std::string json = parameterTokens[1];
            if(id == "*" || (devicesInfo_.find(id) != devicesInfo_.end()))
            {
                templateGet_["device"].SetString(id.c_str(), templateGet_.GetAllocator());
                if(json == "info" || json == "state")
                {
                    templateGet_["json"].SetString(json.c_str(), templateGet_.GetAllocator());
                    success = true;
                }
                else
                    errorMessage = "ERROR: Wrong json type!";	
            }
            else
                errorMessage = "ERROR: Wrong id!";
        }
        else
        {
            errorMessage = "ERROR: Wrong syntax for params!";
        }
    }
    else
        errorMessage = "Not enough params for method";

    return success;
}

void Parser::split(const std::string& s, char delimiter, std::list<std::string>& tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}

void Parser::split(const std::string& s, char delimiter, std::vector<std::string>& tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}