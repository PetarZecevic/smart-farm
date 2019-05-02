#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

class Device
{
public:
    rapidjson::Document info;
    rapidjson::Document state;
    Device(rapidjson::Document& info_, rapidjson::Document& state_)
    {
        info.Swap(info_);
        state.Swap(state_);
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
};

#endif // DEVICE_HPP