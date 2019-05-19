#include <iostream>
//#include <chrono>
#include <unistd.h>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"

#define WAIT_TIME 1000*1000 // one second in microseconds

using namespace std;

class MyReport : public ReportFunction
{
public:
	bool operator()(rapidjson::Document& newState, rapidjson::Document& prevState) override
	{
		bool difference = false;
		/*
		// Measure temperature.
		int temperature = rand() % 30 + 1;
		rapidjson::Value temp;
		temp.SetInt(temperature);
		// If there is difference, signalize and change state.
		if(prevState["temperature"].IsNull() || temperature != prevState["temperature"].GetInt())
		{
			difference = true;
			prevState["temperature"].SetInt(temperature);
			newState.AddMember("temperature", temp, newState.GetAllocator());
		}
		*/
		return difference;
	}
};

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		cout << "Enter name of json document that describes device as argument" << endl;
		return -1;
	}
	else if(argc != 3)
	{
		cout << "Enter name of log file" << endl;
		return -1;
	}
	else
	{
		string jsonDoc(argv[1]);
		srand(time(0));

		IPInfo info;
		if(info.loadDescFromFile(jsonDoc))
			info.setState();
		else
			return -1;

		SSDP_Client c1(info.getByKey("id"), true);
		c1.findGateway();	

		// Start MQTT logic.
		MQTT_Client mqtt_client(info, argv[2]);
		mqtt_client.setLog(c1.getLogTopic());
		
		string loc = c1.getLocation();
		int pos = loc.find_first_of(':', 0); // Get port number from location.
		int port = atoi(loc.substr(pos+1).c_str());
		string ip = loc.substr(0, pos);

		cout << "Ip : " << ip << endl << "Port: " << port << endl;

		if(mqtt_client.connectToBroker(ip, port))
		{
			if(mqtt_client.subscribe())
			{
				mqtt_client.waitFor(2000); // 2 seconds.
				if(mqtt_client.sendInfo())
				{
					cout << "Sent info" << endl;
					mqtt_client.waitFor(500);
					ReportFunction* myReport = new MyReport();
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
	}
	return 0;
}
