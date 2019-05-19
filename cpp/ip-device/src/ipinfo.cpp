#include "ipinfo.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <cstdio>

IPInfo::IPInfo()
{
    description_.Parse("{}");
    state_.Parse("{}");
}

IPInfo::IPInfo(const IPInfo& rhs)
{
    description_.CopyFrom(rhs.description_, description_.GetAllocator());
    state_.CopyFrom(rhs.state_, state_.GetAllocator());
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
            writer.Null();
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

bool IPInfo::loadDescFromFile(const char* filepath)
{
    FILE* fp = fopen(filepath, "r");
    if(fp != NULL)
    {
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        description_.ParseStream(is);
        return !description_.HasParseError();
    }
    else
    {
        return false;
    }
}

bool IPInfo::loadDescFromFile(const std::string filepath)
{
    return loadDescFromFile(filepath.c_str());
}

std::string IPInfo::getDescriptionString()
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    description_.Accept(writer);
    return std::string(s.GetString());
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

std::string IPInfo::getByKey(const std::string key)
{
    return getByKey(key.c_str());
}
