#include <iostream>
//#include <chrono>
#include <unistd.h>
#include <cstdlib>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"

#define WAIT_TIME 1000*1000 // one second in microseconds

using namespace std;

void findGateway(SSDP_Client& cl)
{
	// Trying to find iot controler's.
	while(true)
	{
		if(cl.searchControler())
		{
			// Sent msearch.
			cout << "Sent MSEARCH" << endl;
			usleep(WAIT_TIME);
			if(cl.checkMessages())
			{
				// Found IOT gateway.
				cout << "Found gateway " << endl;
				break;
			}
		}
		else
		{
			// Failed to send msearch, try again.
			usleep(WAIT_TIME);
		}
	}

	return;
}

int main()
{
	IPInfo info;
	info.loadDescFromFile("info.json");
	SSDP_Client c1(info.getByKey("id"), true);

	cout << "JSON : " << info.getDescriptionString() << endl;
	cout << "JSON : " << info.getDescriptionString() << endl;

	findGateway(c1);

	// Start MQTT logic.
	MQTT_Client mqtt_client(info);
	cout << "Location: " << c1.getLocation() << endl;
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
			mqtt_client.waitFor(2000);
			if(mqtt_client.sendInfo())
			{
				cout << "Sent info" << endl;
				mqtt_client.waitFor(500);
				if(mqtt_client.isReportAllowed())
				{
					cout << "Reporting parameters values..." << endl;
				}
			}
		}
	}
	return 0;
}
