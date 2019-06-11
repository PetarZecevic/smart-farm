#include "wiringPi.h"
#include <iostream>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"

const int LED1 = 0;
const int LED2 = 1;

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
	void operator()(rapidjson::Document& newState) override
	{
		return;
	}
};

using namespace std;

int main()
{
	wiringPiSetup();
    pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	
	string jsonDoc("info1.json");
	
	IPInfo info;
	if(info.loadDescFromFile(jsonDoc))
		info.setState();
	else
		return -1;

	//std::cout << info.getStateString() << std::endl;

	
	SSDP_Client c1(info.getByKey("id"), true);
	c1.findGateway();	

	// Start MQTT logic.
	MQTT_Client mqtt_client(info, "mqtt-log-1");
	mqtt_client.setLog(c1.getLogTopic());
	
	string loc = c1.getLocation();
	int pos = loc.find_first_of(':', 0); // Get port number from location.
	int port = atoi(loc.substr(pos+1).c_str());
	string ip = loc.substr(0, pos);

	//cout << "Ip : " << ip << endl << "Port: " << port << endl;

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
				if(mqtt_client.isReportAllowed())
				{
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
