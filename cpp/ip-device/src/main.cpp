#include "ssdp_client.hpp"
#include <iostream>
//#include <chrono>
#include <unistd.h>

#define WAIT_TIME 1000*1000 // one second in microseconds

using namespace std;

int main()
{
	cout << "Hello Raspberry Pi" << endl;
	string name = string("IP1");
	SSDP_Client c1(name, true);
	// Trying to find iot controler's.
	while(true)
	{
		if(c1.searchControler())
		{
			// Sent msearch.
			cout << "Sent MSEARCH" << endl;
			usleep(WAIT_TIME);
			if(c1.checkMessages())
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

	// Start MQTT logic.
	cout << "Topic for logging " << c1.getLogTopic() << endl;
	return 0;
}
