#include "ssdp_manager.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday

#define SUCCESS 0
#define INTERFACE_UPDATE_TIMEOUT 500*1000 // ms

// Public
SSDP_Manager::SSDP_Manager(std::string usn, std::string userId, bool debug)
{
    // Set parameters for client.
    client.sock = 0;
	client.port = 1900;
    client.neighbor_list = NULL;
    client.neighbor_timeout = 5000;
	client.debug = debug;
	strcpy(client.header.search_target, "IP");
	strcpy(client.header.unique_service_name, usn.c_str());
	strcpy(client.header.device_type, "IOTGATEWAY");
    std::string logTopic = userId + "/gateway/" + usn + "/log";
    strcpy(client.header.sm_id, logTopic.c_str());
    
	// Set callbacks.
	lssdp_set_log_callback(log_callback);
	client.network_interface_changed_callback = show_interface_list_and_rebind_socket;
	client.neighbor_list_changed_callback = show_neighbor_list;
	client.packet_received_callback = show_ssdp_packet;
}

SSDP_Manager::SSDP_Manager(const SSDP_Manager& cmanager)
{
    client.sock = cmanager.client.sock;
	client.port = cmanager.client.port;
    client.neighbor_list = NULL; // Don't copy neighbors.
    client.neighbor_timeout = cmanager.client.neighbor_timeout;
	client.debug = cmanager.client.debug;
	strcpy(client.header.search_target, cmanager.client.header.search_target);
	strcpy(client.header.unique_service_name, cmanager.client.header.unique_service_name);
	strcpy(client.header.device_type, cmanager.client.header.device_type);
    strcpy(client.header.sm_id, cmanager.client.header.sm_id);
    
    // Callbacks are not copied.
    lssdp_set_log_callback(log_callback);
    client.network_interface_changed_callback = show_interface_list_and_rebind_socket;
	client.neighbor_list_changed_callback = show_neighbor_list;
	client.packet_received_callback = show_ssdp_packet;
    
}

int SSDP_Manager::setInterface()
{
    return (lssdp_network_interface_update(&client) == SUCCESS) ? 1 : 0;
}

int SSDP_Manager::checkForDevices()
{   
    FD_ZERO(&fs);
   	FD_SET(client.sock, &fs);
    struct timeval tv;
    tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000; // 500 ms
	
    int ret = select(client.sock + 1, &fs, NULL, NULL, &tv);
    if (ret < 0) {
        printf("select error, ret = %d\n", ret);
    }
    if (ret > 0) {
    	ret = lssdp_socket_read(&client);
    }
	return (ret == SUCCESS) ? 1 : 0;
}

void SSDP_Manager::operator()()
{
    while(true)
    {
        if(setInterface())
        {
            bool error = false;
            while(!error)
            {
                if(!checkForDevices())
                    error = true;    
            }
        }
        else
        {
            usleep(INTERFACE_UPDATE_TIMEOUT);
        }
    }
}

SSDP_Manager::~SSDP_Manager()
{
    lssdp_socket_close(&client);    
}

// Private
void SSDP_Manager::log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message)
{
    char level_name[6];
	strcpy(level_name, "DEBUG");
	if (level == LSSDP_LOG_INFO)   strcpy(level_name, "INFO");
	if (level == LSSDP_LOG_WARN)   strcpy(level_name, "WARN");
	if (level == LSSDP_LOG_ERROR)  strcpy(level_name, "ERROR");
	
	printf("[%-5s][%s] %s", level_name, tag, message);
}

int SSDP_Manager::show_interface_list_and_rebind_socket(lssdp_ctx * lssdp)
{
     // 1. show interface list
    printf("\nNetwork Interface List (%zu):\n", lssdp->interface_num);
    size_t i;
    for (i = 0; i < lssdp->interface_num; i++) {
        printf("%zu. %-6s: %s\n",
            i + 1,
            lssdp->interface[i].name,
            lssdp->interface[i].ip
        );
    }
    printf("%s\n", i == 0 ? "Empty" : "");

    // 2. re-bind SSDP socket
    if (lssdp_socket_create(lssdp) != 0) {
        puts("SSDP create socket failed");
        return -1;
    }

    return 0;
}

int SSDP_Manager::show_neighbor_list(lssdp_ctx * lssdp)
{
    int i = 0;
    lssdp_nbr * nbr;
    puts("\nSSDP List:");
    for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
        printf("%d. id = %-9s, ip = %-20s, name = %-12s, device_type = %-8s (%lld)\n",
            ++i,
            nbr->sm_id,
            nbr->location,
            nbr->usn,
            nbr->device_type,
            nbr->update_time
        );
    }
    printf("%s\n", i == 0 ? "Empty" : "");
    return 0;
}

int SSDP_Manager::show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, std::size_t packet_len)
{
    printf("Packet received\n %s", packet);
    return 0;
}