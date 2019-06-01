#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

// Wrapper class around description and state of device.
class Device
{
public:
    rapidjson::Document info;
    rapidjson::Document state;
    Device(rapidjson::Document& info_, rapidjson::Document& state_)
    {
        //TODO: Change Swap to CopyFrom.
        info.CopyFrom(info_, info.GetAllocator());
        state.CopyFrom(state_, state.GetAllocator());
    }

    std::string getInfo()
    {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        info.Accept(writer);
        return std::string(s.GetString());
    }

    std::string getState()
    {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        state.Accept(writer);
        return std::string(s.GetString());
    }

    // Form state JSON based on description JSON, method supports services in description.
    static bool setStateFromDescription(rapidjson::Document& state_, rapidjson::Document& desc_)
    {
        state_.Parse("{}");
        // First two fields are id and group.
        rapidjson::Value::MemberIterator it = desc_.MemberBegin();
        it += 2;
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        for(; it != desc_.MemberEnd(); it++){
            // Store parameters for each service.
            writer.Key(it->name.GetString()); // Service.
            rapidjson::Value& params = desc_[it->name.GetString()];
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
};

#endif // DEVICE_HPP