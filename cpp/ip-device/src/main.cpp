#include <iostream>
//#include <chrono>
#include <unistd.h>
#include <cstdlib>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"

#define WAIT_TIME 1000*1000 // one second in microseconds

using namespace std;

void findControler(SSDP_Client& cl)
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
	string name = string("IP1");
	SSDP_Client c1(name, true);
	
	findControler(c1);

	IPInfo_t info;
	info.id = name;
	info.group = "sensors";

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
		if(mqtt_client.sendInfo())
		{
			cout << "Bravo" << endl;
		}
	}
	/*
	printf("MQTT connected\n");
	rc = client.subscribe(logIpTopic.c_str(), MQTT::QOS2, messageArrived);   
	if (rc != MQTT::returnCode::SUCCESS)
	{
		printf("rc from MQTT subscribe is %d\n", rc);
	}
	else
	{
		MQTT::Message message;
		// QoS 0
		char buf[100];
		sprintf(buf, "Hello World!");
		message.qos = MQTT::QOS0;
		message.retained = false;
		message.dup = false;
		message.payload = (void*)buf;
		message.payloadlen = strlen(buf)+1;
		rc = client.publish(logGatewayTopic.c_str(), message);
		if (rc != 0)
			printf("Error %d from sending QoS 0 message\n", rc);
		else while (arrivedcount == 0)
			client.yield(100);
	*/

	return 0;
}
