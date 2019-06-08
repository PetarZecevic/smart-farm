#include <iostream>
#include <chrono>
#include <unordered_map>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"
#include "parser.hpp"

std::unordered_map<std::string, rapidjson::Document> gDevicesInfo;
std::unordered_map<std::string, rapidjson::Document> gDevicesState;
// id|service_name|parameter_name -> parameter class, map is used as cache for commands.
std::unordered_map<std::string, Parameter*> gParametersInfo;

void parseBrokerLocation(std::string location, std::string& ip, std::string& port);
void parseGatewayId(std::string topic, std::string& gatewayId);
int userInterface();
bool getCommand(Parser& parser, rapidjson::Document& jsonCommand);
void printDevicesInfo();
void printDevicesState();
void printDevices(std::unordered_map<std::string, rapidjson::Document>& devs);
void clearScreen();
void pause();

int main(int argc, char** argv)
{
	std::string userId = "pz97";
	/*
	SSDP_Client ssdpClient(userId, true);
	ssdpClient.findGateway();
	*/
	std::string ip, port, gatewayId;
	/*
	parseBrokerLocation(ssdpClient.getLocation(), ip, port);
	parseGatewayId(ssdpClient.getLogTopic(), gatewayId);
	*/
	ip = "127.0.0.1";
	port = "1883";
	gatewayId = "Gateway1";

	mqtt::connect_options con_opts;
    con_opts.set_connect_timeout(1);
    con_opts.set_user_name(userId);
    con_opts.set_mqtt_version(3);
    con_opts.set_clean_session(true);
	
	MQTT_Client mqttCli(userId, gatewayId, "tcp://" + ip + ":" + port);
	if(mqttCli.connectToBroker(con_opts))
	{
		if(mqttCli.start())
		{
			clearScreen();
			Parser parser(gDevicesInfo, gDevicesState, gParametersInfo);
			bool finished = false;
			while(!finished)
			{
				rapidjson::Document command;
				int option = userInterface();
				if(option != -1)
				{
					switch(option)
					{
						case 1:
							mqttCli.getAllDevicesState(gDevicesState);
							printDevicesState();
							pause();
							break;
						case 2:
							mqttCli.getAllDevicesInfo(gDevicesInfo);
							gParametersInfo.clear();
							printDevicesInfo();
							pause();
							break;
						case 3:
							if(getCommand(parser, command))
								mqttCli.sendCommand(command);
							else
								std::this_thread::sleep_for(std::chrono::seconds(2));
							break;
						case 4:
							finished = true;
							break;
						default:
							break;
					}
				}
				else
				{
					std::cout << "Invalid choice!" << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
				clearScreen();
			}
		}
	}
	return 0;
}

void parseBrokerLocation(std::string location, std::string& ip, std::string& port)
{
	int pos = location.find_first_of(':', 0); // Get port number from location.
	port = location.substr(pos+1);
	ip = location.substr(0, pos);
}

void parseGatewayId(std::string topic, std::string& gatewayId)
{
	int begin,end;
    begin = topic.find_last_of("gateway/");
    end  = topic.find_first_of("/log");
    gatewayId = topic.substr(begin, end-begin);
}

int userInterface()
{
	std::cout << "Options: " << std::endl;
	std::cout << "\t1. Show devices state" << std::endl;
	std::cout << "\t2. Show devices desprition" << std::endl;
	std::cout << "\t3. Send command" << std::endl;
	std::cout << "\t4. Exit" << std::endl;
	std::cout << ":: ";

	int option;
	std::string optionstr;
	std::getline(std::cin, optionstr);
	try
	{
		std::stringstream(optionstr) >> option;	
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
	
	if(option < 1 || option > 4)
		option = -1;

	return option;
}

bool getCommand(Parser& parser, rapidjson::Document& jsonCommand)
{
	std::cout << "command> ";
	std::string command;
	std::getline(std::cin, command);
	std::cout.flush();

	std::string errorMessage;
	if(parser.parseCommand(command, jsonCommand, errorMessage))
		return true;
	else
	{
		std::cout << errorMessage << std::endl;
		return false;
	}
}

void printDevicesInfo()
{
	printDevices(gDevicesInfo);
}

void printDevicesState()
{
	printDevices(gDevicesState);
}

void printDevices(std::unordered_map<std::string, rapidjson::Document>& devs)
{
	// Print devices by their groups.
	std::unordered_map<std::string, std::string> groups;
	for(auto it = devs.begin(); it != devs.end(); it++)
	{
		std::string group(it->second["group"].GetString());
		auto fit = groups.find(group);
		if(fit == groups.end())
		{
			// Init group.
			groups[group] = "";
		}
		// Group initialized already.
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
		it->second.Accept(writer);
		
		groups[group] += s.GetString();
		groups[group] += "\n";
	}

	for(auto it = groups.begin(); it != groups.end(); it++)
	{
		std::cout << it->first << " -> " << std::endl;
		std::cout << it->second << std::endl;
		std::cout << "------------------------------" << std::endl;
	}
}

void clearScreen()
{
	system("clear");
}

void pause()
{
	std::cout << "Press any key to continue..." << std::endl;
	std::string key;
	std::getline(std::cin, key);
}
