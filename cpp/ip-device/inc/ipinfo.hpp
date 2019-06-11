#ifndef IP_INFO_HPP
#define IP_INFO_HPP

#include <rapidjson/document.h>

/**
 * Class that describes all features of ip device.
 * Represents device description and state as JSON-DOM object.
 */
class IPInfo
{
public:
    /**
     * Default constructor, set description and state to empty json.
     */
    IPInfo();

    /**
     * Copy constructor, copy description and state json from input.
     * @param rhs
     * IPInfo instance to copy from.
     */
    IPInfo(const IPInfo& rhs);
        
    /**
     * Create JSON DOM that represents state of device from device description.
     * @return
     * True if parsing is done succesfully, otherwise False.
     */
    bool setState();
    
    /**
     * Load device description from json file, parse it and store in JSON DOM.
     * @param filepath
     * String that represents json file location.
     * @return
     * True if parsing and loading file is correct, otherwise False.
     */
    bool loadDescFromFile(const char* filepath);
    
    /**
     * Overloaded for version for cpp string.
     */
    bool loadDescFromFile(const std::string filepath);
    /**
     * Merge parameter values from @param newState with current state.
     * Method assumes that @param newState is correctly formed based on JSON state structure.
     */
    bool mergeState(rapidjson::Document& newState);
    rapidjson::Document& getDescriptionDOM() { return description_;}
    rapidjson::Document& getStateDOM() { return state_;}
    std::string getDescriptionString();
    std::string getStateString();
    std::string getByKey(const char* key);
    std::string getByKey(const std::string key);
private:
    rapidjson::Document description_;
    rapidjson::Document state_;
};

#endif // IP_INFO_HPP
