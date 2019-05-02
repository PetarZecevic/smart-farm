#include "ssdp_client.hpp"
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday

#define SUCCESS 0
#define WAIT_TIME 1000*1000 // microseconds

// Public
SSDP_Client::SSDP_Client(std::string usn, bool debug)
{
	// Set parameters for client.
	client_.port = 1900;
	client_.debug = debug;
	strcpy(client_.header.search_target, "IOTGATEWAY");
	strcpy(client_.header.unique_service_name, usn.c_str());
	strcpy(client_.header.device_type, "IP");
    client_.neighbor_list = NULL; // Added because segmentation fault.
	// Set callbacks.
	lssdp_set_log_callback(this->log_callback);
	client_.network_interface_changed_callback = this->show_interface_list_and_rebind_socket;
	client_.neighbor_list_changed_callback = this->show_neighbor_list;
	client_.packet_received_callback = this->show_ssdp_packet;
    // Remember usn.
    this->usn_ = usn;
}

// It's assumes that socket is configured and opened.
bool SSDP_Client::checkMessages()
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
        if(ret == SUCCESS)
        {
            lssdp_nbr* neighbor = client_.neighbor_list;
            gatewayLogTopic_ = std::string(neighbor->sm_id);
            gatewayLocation_ = std::string(neighbor->location);
        }
    }
    else
    {
        ret = -1;
    }
	return (ret == SUCCESS);
}

// Sends MSEARCH message to find controler.
bool SSDP_Client::searchControler()
{
	int ret;
	if((ret = lssdp_network_interface_update(&client_)) == SUCCESS)
	{
        if(rebindSocket())
		    ret = lssdp_send_msearch(&client_);
	}
	return (ret == SUCCESS);
}

void SSDP_Client::findGateway()
{
    // Trying to find iot controler's.
	while(true)
	{
		if(searchControler())
		{
			usleep(WAIT_TIME);
			if(checkMessages())
			{
				// Found IOT gateway.
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

// Private
void SSDP_Client::log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message) 
{
	char level_name[6];
	strcpy(level_name, "DEBUG");
	if (level == LSSDP_LOG_INFO)   strcpy(level_name, "INFO");
	if (level == LSSDP_LOG_WARN)   strcpy(level_name, "WARN");
	if (level == LSSDP_LOG_ERROR)  strcpy(level_name, "ERROR");
	
	printf("[%-5s][%s] %s", level_name, tag, message);    
}

bool SSDP_Client::rebindSocket()
{
    bool ret = true;
    if (lssdp_socket_create(&client_) != SUCCESS) {
        puts("SSDP create socket failed");
        ret = false;
    }
    return ret;
}

int SSDP_Client::show_interface_list_and_rebind_socket(lssdp_ctx * lssdp) {
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
    return 0;
}

int SSDP_Client::show_neighbor_list(lssdp_ctx * lssdp) {
    int i = 0;
    lssdp_nbr * nbr;
    puts("\nSSDP List:");
    for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
        printf("%d. id = %-9s\n ip = %-20s\n name = %-12s\n device_type = %-8s\n (%lld)\n",
            ++i,
            nbr->sm_id,
            nbr->location,
            nbr->usn,
            nbr->device_type,
            nbr->update_time
        );
    }
    return 0;
}

int SSDP_Client::show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, std::size_t packet_len) {
    printf("Packet received\n %s", packet);
    return 0;
}


