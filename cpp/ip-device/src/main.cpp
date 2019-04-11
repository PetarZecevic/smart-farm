#include "ssdp_client.hpp"
#include <iostream>
//#include <chrono>
#include <unistd.h>

#define MQTTCLIENT_QOS2 1
#include <memory.h>
#include "MQTTClient.h"
#define DEFAULT_STACK_SIZE -1
#include "linux.cpp"

#define WAIT_TIME 1000*1000 // one second in microseconds

using namespace std;

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;

    printf("Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		++arrivedcount, message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
}

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
	cout << "MQTT local server location " << c1.getLocation() << endl;
	cout << "Topic for logging " << c1.getLogTopic() << endl;
	
	
	IPStack ipstack = IPStack();
	MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);
	
	string hostname = c1.getLocation();
    int port = 1883;
    printf("Connecting to %s:%d\n", hostname.c_str(), port);
    int rc = ipstack.connect(hostname.c_str(), port);
	if (rc != MQTT::returnCode::SUCCESS)
	{
		printf("Failed to connect\n");
	    printf("rc from TCP connect is %d\n", rc);
	}
	else
	{
		printf("MQTT connecting\n");
    	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    	data.MQTTVersion = 3;
    	data.clientID.cstring = (char*)"ip_device1";
    	rc = client.connect(data);
		if (rc != MQTT::returnCode::SUCCESS)
		{
	    	printf("rc from MQTT connect is %d\n", rc);
		}	
		else
		{
			printf("MQTT connected\n");
			rc = client.subscribe("hello/topic", MQTT::QOS2, messageArrived);   
			if (rc != MQTT::returnCode::SUCCESS)
			{
				printf("rc from MQTT subscribe is %d\n", rc);
			}
			else
			{
				MQTT::Message message;

				// QoS 0
				char buf[100];
				sprintf(buf, "Hello World!  QoS 0 message from app version %f", 1.0);
				message.qos = MQTT::QOS0;
				message.retained = false;
				message.dup = false;
				message.payload = (void*)buf;
				message.payloadlen = strlen(buf)+1;
				rc = client.publish("hello/topic", message);
				if (rc != 0)
					printf("Error %d from sending QoS 0 message\n", rc);
				else while (arrivedcount == 0)
					client.yield(100);
			}	
		}
	}
	return 0;
}
