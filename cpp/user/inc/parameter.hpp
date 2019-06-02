#ifndef PARAMETER_HPP
#define PARAMETER_HPP

#include <string>
#include <vector>

enum class ValuesType{RANGE, LIST};

// Base class that encapsulates common features for all parameters.
class Parameter
{
protected:
    bool writePermission_;
    ValuesType valuesType_;
public:
    bool isWriteAllowed() const { return writePermission_;}
    ValuesType getValuesType() const { return valuesType_;}
    /**
     * Checks if parameter can be set to given value, based on permitted values. 
    */
    virtual bool isValueAllowed(std::string value) = 0;
};

// Assumption is that values are integer numbers.
class RangeParameter : public Parameter
{
private:
    int minimumValue, maximumValue;
public:
    RangeParameter(int minValue, int maxValue, bool writeAllowed):
        minimumValue(minValue),
        maximumValue(maxValue)
    {
        writePermission_ = writeAllowed;
        valuesType_ = ValuesType::RANGE;     
    }

    bool isValueAllowed(std::string value) override
    {
        int numValue;
        try{
            numValue = std::stoi(value);
        }catch(std::exception& e){
            // Conversion error.
            return false;
        }

        if(numValue < minimumValue || numValue > maximumValue)
            return false;
        else
            return true;
    }
};

class ListParameter : public Parameter
{
private:
    std::vector<std::string> valuesList_;
public:
    ListParameter(std::vector<std::string> vals, bool writeAllowed):
        valuesList_(vals)
    {
        writePermission_ = writeAllowed;
        valuesType_ = ValuesType::LIST;
    }

    bool isValueAllowed(std::string value) override
    {
        bool found = false;
        for(std::string val : valuesList_)
        {
            if(val == value)
            {
                found = true;
                break;
            }
        }
        return found;
    }
};

#endif