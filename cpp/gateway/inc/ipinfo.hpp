#ifndef IP_INFO_HPP
#define IP_INFO_HPP

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

// Class that describes all features of ip device.
// Handels device description as JSON-DOM object.
class IPInfo
{
public:
    void copy(IPInfo& rhs);
    bool setDescription(std::string desc); // Set DOM from string.
    bool loadDescFromFile(std::string filepath);
    bool loadDescFromFile(const char* filepath);
    rapidjson::Document& getDescriptionDOM() { return description_;}
    std::string getDescriptionString();
    std::string getByKey(const std::string key);
    std::string getByKey(const char* key);
private:
    rapidjson::Document description_;
    std::string strDesc_;
};


#endif // IP_INFO_HPP
