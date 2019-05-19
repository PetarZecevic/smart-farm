#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstdio>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include "ssdp_client.hpp"
#include "mqtt_client.hpp"

std::unordered_map<std::string, rapidjson::Document> gDevicesInfo;
std::unordered_map<std::string, rapidjson::Document> gDevicesState;

void parseBrokerLocation(std::string location, std::string& ip, std::string& port);
void parseGatewayId(std::string topic, std::string& gatewayId);
int userInterface();
bool parseCommand(std::vector<std::string>& tokens, rapidjson::Document& jsonCommand);
bool getCommand(rapidjson::Document& jsonCommand);
std::string commandSetTemplate();
std::string commandGetTemplate();
void split(const std::string& s, char delimiter, std::vector<std::string>& tokens);
void printDevicesInfo();
void printDevicesState();
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
							printDevicesInfo();
							pause();
							break;
						case 3:
							if(getCommand(command))
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

bool getCommand(rapidjson::Document& jsonCommand)
{
	std::cout << "command> ";
	std::string command;
	std::getline(std::cin, command);
	std::cout.flush();
	
	std::vector<std::string> tokens;
	split(command, ' ', tokens);

	return parseCommand(tokens, jsonCommand);
}

bool parseCommand(std::vector<std::string>& tokens, rapidjson::Document& jsonCommand)
{
	bool success = false;
	// For now implement set command.
	if(!tokens.empty())
	{
		std::string opcode = tokens[0];
		if(opcode == "SET")
		{	

			std::string commandTemp = commandSetTemplate();
			jsonCommand.Parse(commandTemp.c_str());
			jsonCommand["command_type"].SetString("SET");
			// SET device_id.param value
			if(tokens.size() == 3)
			{
				std::vector<std::string> parameterTokens;
				split(tokens[1], '.', parameterTokens);
				if(parameterTokens.size() == 2)
				{
					// device_id.param
					std::string id = parameterTokens[0];
					std::string param = parameterTokens[1];

					auto it = gDevicesInfo.find(id);
					if(it != gDevicesInfo.end())
					{
						// Found device.
						bool found = false;
						rapidjson::Value paramArray = it->second["parameters"].GetArray();
						for(rapidjson::SizeType i = 0; i < paramArray.Size(); i++)
						{
							if(paramArray[i].IsString())
							{
								std::string tmp(paramArray[i].GetString());
								if(tmp == param)
								{
									// Found parameter.
									found = true;
									// Check matching between value and param type.
									jsonCommand["group"].SetString(it->second["group"].GetString(), jsonCommand.GetAllocator());
									jsonCommand["device"].SetString(id.c_str(), jsonCommand.GetAllocator());
									jsonCommand["parameter"].SetString(param.c_str(), jsonCommand.GetAllocator());
									jsonCommand["value"].SetString(tokens[2].c_str(), jsonCommand.GetAllocator());
						
								}
							}
						}	
						if(!found)
						{
							std::cout << "ERROR: Parameter " << param << " not found." << std::endl;
						}
						else
						{
							success = true;	
						}
						
					}
					else
					{
						std::cout << "ERROR: Device " << id << " not found." << std::endl;
					}					
				}
				else
				{
					std::cout << "ERROR: Expected deviceId.paramName expression, " << tokens[1]  << " given." << std::endl;
				}
			}
			else
			{
				std::cout << "ERROR: 2 params expected, " << tokens.size()-1 << " given." << std::endl;
			}
		}
		else if(opcode == "GET")
		{
			std::string commandTemp = commandGetTemplate();
			jsonCommand.Parse(commandTemp.c_str());
			jsonCommand["command_type"].SetString("GET");
			if(tokens.size() == 2)
			{
				// possible commands with this opcode.
				// GET *.info
				// GET *.state
				// GET id.state
				// GET id.info
				std::vector<std::string> parameterTokens;
				split(tokens[1], '.', parameterTokens);
				if(parameterTokens.size() == 2)
				{
					std::string id = parameterTokens[0];
					std::string json = parameterTokens[1];
					if(id == "*" || (gDevicesInfo.find(id) != gDevicesInfo.end()))
					{
						jsonCommand["device"].SetString(id.c_str(), jsonCommand.GetAllocator());
						if(json == "info" || json == "state")
						{
							jsonCommand["json"].SetString(json.c_str(), jsonCommand.GetAllocator());
							success = true;
						}
						else
						{
							std::cout << "ERROR: Wrong json type!" << std::endl;
						}	
					}
					else
					{
						std::cout << "ERROR: Wrong id!" << std::endl;
					}
				}
				else
				{
					std::cout << "ERROR: Wrong syntax for params!" << std::endl;
				}
			}
			else
			{
				std::cout << "ERROR: Not enough params!" << std::endl;
			}		
		}
		else
		{
			std::cout << "ERROR: Method not valid!" << std::endl;
		}
	}
	else
	{
		std::cout << "ERROR: Empty input." << std::endl;
	}
		
	return success;
}

std::string commandSetTemplate()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key("command_type"); writer.Null();
	writer.Key("group"); writer.Null();
	writer.Key("device"); writer.Null();
	writer.Key("parameter"); writer.Null();
	writer.Key("value"); writer.Null();
	writer.EndObject();
	return std::string(s.GetString());
}

std::string commandGetTemplate()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key("command_type"); writer.Null();
	writer.Key("json"); writer.Null();
	writer.Key("device"); writer.Null();
	writer.EndObject();
	return std::string(s.GetString());
}

void split(const std::string& s, char delimiter, std::vector<std::string>& tokens)
{
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}

void printDevicesInfo()
{
	// Print devices by their groups.
	std::unordered_map<const char*, std::string> groups;
	for(auto it = gDevicesInfo.begin(); it != gDevicesInfo.end(); it++)
	{
		auto fit = groups.find(it->second["group"].GetString());
		if(fit == groups.end())
		{
			// Init group.
			groups[it->second["group"].GetString()] = "";
		}
		// Group initialized already.
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
		it->second.Accept(writer);
		
		groups[it->second["group"].GetString()] += s.GetString();
		groups[it->second["group"].GetString()] += "\n";
	}

	for(auto it = groups.begin(); it != groups.end(); it++)
	{
		std::cout << it->first << " -> " << std::endl;
		std::cout << it->second << std::endl;
		std::cout << "------------------------------" << std::endl;
	}
}

void printDevicesState()
{
	// Print devices by their groups.
	std::unordered_map<const char*, std::string> groups;
	for(auto it = gDevicesState.begin(); it != gDevicesState.end(); it++)
	{
		auto fit = groups.find(it->second["group"].GetString());
		if(fit == groups.end())
		{
			// Init group.
			groups[it->second["group"].GetString()] = "";
		}
		// Group initialized already.
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
		it->second.Accept(writer);
		
		groups[it->second["group"].GetString()] += s.GetString();
		groups[it->second["group"].GetString()] += "\n";
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