#ifndef IP_INFO_HPP
#define IP_INFO_HPP

#include <rapidjson/document.h>

// Class that describes all features of ip device.
// Handels device description as JSON-DOM object.
class IPInfo
{
public:
    void copy(IPInfo& rhs);
    bool setDescription(std::string desc); // Set DOM from string.
    bool setState(); // Set state from description string.
    bool loadDescFromFile(std::string filepath);
    bool loadDescFromFile(const char* filepath);
    rapidjson::Document& getDescriptionDOM() { return description_;}
    rapidjson::Document& getStateDOM() { return state_;}
    std::string getDescriptionString();
    std::string getByKey(const std::string key);
    std::string getByKey(const char* key);
private:
    rapidjson::Document description_;
    rapidjson::Document state_;
    std::string strDesc_;
};


#endif // IP_INFO_HPP
