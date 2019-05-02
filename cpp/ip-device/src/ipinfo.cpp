#include "ipinfo.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

void IPInfo::copy(IPInfo& rhs)
{
    description_.Swap(rhs.getDescriptionDOM());
    state_.Swap(rhs.getStateDOM());
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

bool IPInfo::setState()
{
    state_.Parse("{}");
    const rapidjson::Value& params = description_["parameters"];
    if(params.IsArray())
    {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        for(rapidjson::SizeType i = 0; i < params.Size(); i++)
        {
            writer.Key(params[i].GetString());
            writer.Int(0);
        }
        writer.EndObject();
        state_.Parse(s.GetString());
        if(state_.HasParseError())
        {
            state_.Parse("{}");
            return false;
        }
        else
            return true;
    }
    else
        return false;
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