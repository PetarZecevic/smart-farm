#include "ssdp_manager.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <unistd.h>     // select

#define SUCCESS 0
#define INTERFACE_UPDATE_TIMEOUT 2000 // ms
#define CHECK_DEVICES_TIMEOUT 1000 // ms
#define REBIND_TIMEOUT 1000 // ms

// Public
SSDP_Manager::SSDP_Manager(std::string usn, std::string userId, int brokerPort, bool debug)
{
    // Set parameters for client.
    client_.sock = 0;
	client_.port = 1900;
    client_.neighbor_list = NULL;
    client_.neighbor_timeout = 5000;
	client_.debug = debug;
	strcpy(client_.header.search_target, "IP");
	strcpy(client_.header.unique_service_name, usn.c_str());
	strcpy(client_.header.device_type, "IOTGATEWAY");
    std::string logTopic = userId + "/gateway/" + usn + "/log";
    strcpy(client_.header.sm_id, logTopic.c_str());
    memset(client_.header.location.prefix, '\0', LSSDP_FIELD_LEN);
    memset(client_.header.location.domain, '\0', LSSDP_FIELD_LEN);
    sprintf(client_.header.location.suffix, ":%s", std::to_string(brokerPort).c_str());
	// Set callbacks.
	lssdp_set_log_callback(log_callback);
	client_.network_interface_changed_callback = show_interface_list_and_rebind_socket;
	client_.neighbor_list_changed_callback = show_neighbor_list;
	client_.packet_received_callback = show_ssdp_packet;
}

SSDP_Manager::SSDP_Manager(const SSDP_Manager& cmanager)
{
    client_.sock = cmanager.client_.sock;
	client_.port = cmanager.client_.port;
    client_.neighbor_list = NULL; // Don't copy neighbors.
    client_.neighbor_timeout = cmanager.client_.neighbor_timeout;
	client_.debug = cmanager.client_.debug;
	strcpy(client_.header.search_target, cmanager.client_.header.search_target);
	strcpy(client_.header.unique_service_name, cmanager.client_.header.unique_service_name);
	strcpy(client_.header.device_type, cmanager.client_.header.device_type);
    strcpy(client_.header.sm_id, cmanager.client_.header.sm_id);
    strcpy(client_.header.location.prefix, cmanager.client_.header.location.prefix);
    strcpy(client_.header.location.domain, cmanager.client_.header.location.domain);
    strcpy(client_.header.location.suffix, cmanager.client_.header.location.suffix);

    // Callbacks are not copied.
    lssdp_set_log_callback(log_callback);
    client_.network_interface_changed_callback = show_interface_list_and_rebind_socket;
	client_.neighbor_list_changed_callback = show_neighbor_list;
	client_.packet_received_callback = show_ssdp_packet;
}

bool SSDP_Manager::setInterface()
{
    return (lssdp_network_interface_update(&client_) == SUCCESS);
}

bool SSDP_Manager::checkForDevices()
{   
    FD_ZERO(&fs_);
   	FD_SET(client_.sock, &fs_);
    struct timeval tv;
    tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000; // 500 ms
	
    int ret = select(client_.sock + 1, &fs_, NULL, NULL, &tv);
    if (ret < 0) {
        printf("select error, ret = %d\n", ret);
    }
    if (ret > 0) {
    	ret = lssdp_socket_read(&client_);
    }
    return (ret == SUCCESS);
}

bool SSDP_Manager::rebindSocket()
{
    bool ret = true;
    if (lssdp_socket_create(&client_) != SUCCESS) 
    {
        std::string logM("SSDP create socket failed");
        recordLog(logM);
        ret = false;
    }
    return ret;
}

void SSDP_Manager::operator()()
{
    const int REBIND_TRY = 3;

    while(true)
    {
        if(setInterface())
        {
            bool rebindSuccess = false;
            // Try few times to rebind socket.
            for(int i = 0; i < REBIND_TRY; i++)
            {
                if(rebindSocket())
                {
                    rebindSuccess = true;
                    break;
                }
                else
                    usleep(REBIND_TIMEOUT * 1000);
                
            }
            if(rebindSuccess)
            {
                // Check for messages in loop.
                while(true)
                {
                    if(!checkForDevices())
                        break;
                    else
                        usleep(CHECK_DEVICES_TIMEOUT * 1000);
                }    
            }
            else
                // End ssdp manager-loop if rebind constantly failes.
                break;
        }
        else
        {
            usleep(INTERFACE_UPDATE_TIMEOUT * 1000);
        }
    }
}

SSDP_Manager::~SSDP_Manager()
{
    lssdp_socket_close(&client_);   
}

// Private
void SSDP_Manager::recordLog(const std::string& logMessage)
{
    static std::fstream logFile;
    if(logFile.is_open())
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        logFile << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << " -> ";
        logFile << logMessage << std::endl;
    }
    else
    {
        // Set log file.
        try{
            logFile.open("log/ssdp-log.txt", std::ios::openmode::_S_out);
        }catch(std::exception& e){
            std::cout << "Unable to open ssdp log file." << std::endl;
        }
    }
}

void SSDP_Manager::log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message)
{
    std::string level_name;
	level_name = "DEBUG";
	if (level == LSSDP_LOG_INFO)   level_name = "INFO";
	if (level == LSSDP_LOG_WARN)   level_name = "WARN";
	if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";
	
    std::string logM = "";
    logM += "[" + level_name + "]" + "[" + tag + "]" + " " + message;
 	recordLog(logM);
}

int SSDP_Manager::show_interface_list_and_rebind_socket(lssdp_ctx * lssdp)
{
     // 1. show interface list
    std::stringstream builder;
    builder << "\nNetwork Interface List (%zu):\n" << lssdp->interface_num;

    size_t i;
    for (i = 0; i < lssdp->interface_num; i++) {
        builder << i+1 << ". ";
        builder << lssdp->interface[i].name << ": ";
        builder << lssdp->interface[i].ip << std::endl;

        /*
        printf("%zu. %-6s: %s\n",
            i + 1,
            lssdp->interface[i].name,
            lssdp->interface[i].ip
        );
        */
    }
    if(i == 0)
        builder << "Empty";

    std::string logM = builder.str();
    recordLog(logM);
    return 0;
}

int SSDP_Manager::show_neighbor_list(lssdp_ctx * lssdp)
{
    int i = 0;
    lssdp_nbr * nbr;
    std::stringstream builder;
    builder << "\nSSDP List:";
    for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
        builder << ++i << ". ";
        builder << "id = " << nbr->sm_id << ", ";
        builder << "ip = " << nbr->location << ", ";
        builder << "name = " << nbr->device_type << ", ";
        builder << "device_type = " << nbr->device_type << " ";
        builder << "(" << nbr->update_time << ")" << std::endl;
        /*
        printf("%d. id = %-9s, ip = %-20s, name = %-12s, device_type = %-8s (%lld)\n",
            ++i,
            nbr->sm_id,
            nbr->location,
            nbr->usn,
            nbr->device_type,
            nbr->update_time
        );
        */
    }
    if(i == 0)
        builder << "Empty";
        
    //printf("%s\n", i == 0 ? "Empty" : "");
    std::string logM = builder.str();
    recordLog(logM);
    return 0;
}

int SSDP_Manager::show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, std::size_t packet_len)
{
    std::string logM = "Packet received\n";
    logM += packet;
    recordLog(logM);
    return 0;
}