#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <unordered_map>
#include <list>
#include <rapidjson/document.h>
#include "parameter.hpp"

// Class for parsing user commands.
class Parser
{
public:
    Parser(std::unordered_map<std::string, rapidjson::Document>& dInfo,
        std::unordered_map<std::string, rapidjson::Document>& dState,
        std::unordered_map<std::string, Parameter*>& pInfo);
    bool parseCommand(std::string command, rapidjson::Document& jsonCommand, std::string& errorMessage);
private:
    bool methodSet(std::list<std::string>& tokens, std::string& errorMessage);
    bool methodGet(std::list<std::string>& tokens, std::string& errorMessage);
    void split(const std::string& s, char delimiter, std::list<std::string>& tokens);
    void split(const std::string& s, char delimiter, std::vector<std::string>& tokens);
    rapidjson::Document templateSet_; // JSON form of set command.
    rapidjson::Document templateGet_; // JSON form of get command.
    std::unordered_map<std::string, rapidjson::Document>& devicesInfo_;
    std::unordered_map<std::string, rapidjson::Document>& devicesState_;
    std::unordered_map<std::string, Parameter*>& paramsInfo_;
};

#endif // PARSER_HPP