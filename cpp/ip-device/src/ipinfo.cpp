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
    // First two fields are id and group.
    rapidjson::Value::MemberIterator it = description_.MemberBegin();
    it += 2;
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    writer.StartObject();
    for(; it != description_.MemberEnd(); it++){
        // Store parameters for each service.
        writer.Key(it->name.GetString()); // Service.
        rapidjson::Value& params = description_[it->name.GetString()];
        if(params.IsObject())
        {
            writer.StartObject();
            for(rapidjson::Value::MemberIterator pit = params.MemberBegin(); pit != params.MemberEnd(); pit++)
            {
                writer.Key(pit->name.GetString()); // Parameter.
                writer.Null();
            }
            writer.EndObject();
        }
        else
            return false;
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

bool IPInfo::mergeState(rapidjson::Document& newState)
{
    // Types of all possible values from JSON document.
    static std::string kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    rapidjson::Document::AllocatorType& allocator = state_.GetAllocator();
    // Iterate through all services.
    for (rapidjson::Value::ConstMemberIterator itr = newState.MemberBegin(); itr != newState.MemberEnd(); ++itr)
    {
        rapidjson::Value& params = newState[itr->name.GetString()];
        if(params.IsObject())
        {
            // Iterate through service parameters.
            for(rapidjson::Value::MemberIterator pit = params.MemberBegin(); pit != params.MemberEnd(); pit++)
            {
                rapidjson::Value& v = state_[itr->name.GetString()][pit->name.GetString()];
                if(kTypeNames[pit->value.GetType()] == "String")
                {
                    v.SetString(pit->value.GetString(), allocator);
                }
                else if(kTypeNames[pit->value.GetType()] == "Number")
                {
                    // Support int and double.
                    if(pit->value.IsInt())
                    {
                        v.SetInt(pit->value.GetInt());
                    }
                    else if(pit->value.IsDouble())
                    {
                        v.SetDouble(pit->value.GetDouble());
                    }
                }
            }
        }
        else
            return false;
    }
    return true;
}

std::string IPInfo::getDescriptionString()
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    description_.Accept(writer);
    return std::string(s.GetString());
}

std::string IPInfo::getStateString()
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    state_.Accept(writer);
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
