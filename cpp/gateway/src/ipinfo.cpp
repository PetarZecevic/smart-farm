#include "ipinfo.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>

void IPInfo::copy(IPInfo& rhs)
{
    description_.Swap(rhs.getDescriptionDOM());
    strDesc_ = rhs.strDesc_;
}

bool IPInfo::setDescription(std::string desc)
{
    bool ret = true;
    description_.Parse(desc);
    ret = description_.HasParseError();
    if(!ret)
        strDesc_ = desc;
    return !ret;
}

bool IPInfo::loadDescFromFile(std::string filepath)
{
    std::basic_string<char> f_desc = "{\"id\":\"ip_device_1\",\"group\":\"sensors\",\"parameters\":[\"temperature\"]}";
    return setDescription(f_desc);
}

bool IPInfo::loadDescFromFile(const char* filepath)
{
    std::string f_desc = "{\"id\":\"ip_device_1\",\"group\":\"sensors\",\"parameters\":[\"temperature\"]}";
    return setDescription(f_desc);
}

std::string IPInfo::getDescriptionString()
{
    return strDesc_;
}

// Prepravi ovo tako da bude genericno, genericna provera tipa i genericna povratna vrednost. 
std::string IPInfo::getByKey(const std::string key)
{
    if(description_.IsObject())
    {
        if(description_.HasMember(key.c_str()) && description_[key.c_str()].IsString())
        {
            return description_[key.c_str()].GetString();
        }
    }
    return std::string("");
}

// Ovo takodje.
std::string IPInfo::getByKey(const char* key)
{
    if(description_.IsObject())
    {
        if(description_.HasMember(key) && description_[key].IsString())
        {
            return description_[key].GetString();
        }
    }
    return std::string("");
}