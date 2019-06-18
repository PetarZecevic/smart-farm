#include "wiringPi.h"
#include <iostream>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

const int LED1 = 17;
const int LED2 = 18;

class BulbReport : public ReportFunction
{
public:
	bool operator()(rapidjson::Document& newState, rapidjson::Document& prevState) override
	{
		bool difference = false;
		return difference;
	}
};

class BulbUpdate : public UpdateFunction
{
public:
	void operator()(rapidjson::Document& state, rapidjson::Document& newState) override
	{
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> w(s);

		state.Accept(w);
		std::cout << s.GetString() << std::endl;
		
		// Check LED1 state.
		if(newState.HasMember("ContactService"))
		{
			if(strcmp((const char*)newState["ContactService"]["State"].GetString(), "T") == 0)
				digitalWrite(LED1, HIGH);
			else
				digitalWrite(LED1, LOW);
		}
		if(newState.HasMember("LightService"))
		{	
			if(newState["LightService"].HasMember("DimLevel"))
			{
				std::cout << "Entered DimLevel check" << std::endl;
				if(/*state["LightService"]["State"].IsString() &&*/
				   state["ContactService"]["State"].IsString())
				{
					if(/*(strcmp((const char*) state["LightService"]["State"].GetString(), "ON") == 0) &&*/ 
				       (strcmp((const char*) state["ContactService"]["State"].GetString(), "T") == 0))
					{
						int intensity = std::stoi(newState["LightService"]["DimLevel"].GetString());
						// Convert 0-100 range to 0-1023.
						// Library accepts 0-1023 range for intensity represented as pulse-width-modulation.
						intensity = intensity * 10;
						pwmWrite(LED2, intensity);
					}
				}
			}	
			if(newState["LightService"].HasMember("State"))
			{
				if(strcmp(newState["LightService"]["State"].GetString(), "OFF") == 0)
					pwmWrite(LED2, 0); // Turn of light.
			}
		}
	}
};

using namespace std;

int main()
{
	wiringPiSetupGpio();
    pinMode(LED1, OUTPUT); // Contact
	pinMode(LED2, PWM_OUTPUT); // Light

	// Reset LED's.
	digitalWrite(LED1, LOW);
	pwmWrite(LED2, 0);

	string jsonDoc("info1.json");
	
	IPInfo info;
	if(info.loadDescFromFile(jsonDoc))
		info.setState();
	else
		return -1;
	
	SSDP_Client c1(info.getByKey("id"), true);
	c1.findGateway();	

	// Start MQTT logic.
	MQTT_Client mqtt_client(info, "mqtt-log-1");
	mqtt_client.setLog(c1.getLogTopic());
	
	string loc = c1.getLocation();
	int pos = loc.find_first_of(':', 0);
	int port = atoi(loc.substr(pos+1).c_str());
	string ip = loc.substr(0, pos);

	if(mqtt_client.connectToBroker(ip, port))
	{
		if(mqtt_client.subscribe())
		{
			mqtt_client.waitFor(2000); // 2 seconds.
			if(mqtt_client.sendInfo())
			{
				cout << "Sent info" << endl;
				mqtt_client.waitFor(500);
				ReportFunction* myReport = new BulbReport();
				UpdateFunction* myUpdate = new BulbUpdate();
				if(mqtt_client.isReportAllowed())
				{
					mqtt_client.setUpdateFunction(myUpdate);
					cout << "Reporting parameters values..." << endl;
					// Send JSON report state.
					mqtt_client.report(myReport);
				}
				delete myReport;
			}
		}
	}
	return 0;
}
