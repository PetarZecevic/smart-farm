#include "ssdp_client.hpp"
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday

#define SUCCESS 0

// Public
SSDP_Client::SSDP_Client(std::string usn, bool debug)
{
	// Set parameters for client.
	client.port = 1900;
	client.debug = debug;
	strcpy(client.header.search_target, "IOTGATEWAY");
	strcpy(client.header.unique_service_name, usn.c_str());
	strcpy(client.header.device_type, "IP");
	// Set callbacks.
	lssdp_set_log_callback(this->log_callback);
	client.network_interface_changed_callback = this->show_interface_list_and_rebind_socket;
	client.neighbor_list_changed_callback = this->show_neighbor_list;
	client.packet_received_callback = this->show_ssdp_packet;
}

// It's assumes that socket is configured and opened.
int SSDP_Client::checkMessages()
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
        if(ret == SUCCESS)
        {
            lssdp_nbr* neighbor = client.neighbor_list;
            gatewayLogTopic = std::string(neighbor->sm_id);
        }
    }
    else
    {
        ret = -1;
    }

	return (ret == SUCCESS) ? 1 : 0;
}

// Sends MSEARCH message to find controler.
int SSDP_Client::searchControler()
{
	int ret;
	if((ret = lssdp_network_interface_update(&client)) == SUCCESS)
	{
		ret = lssdp_send_msearch(&client);
	}
	return (ret == SUCCESS) ? 1 : 0;
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

    // 2. re-bind SSDP socket
    if (lssdp_socket_create(lssdp) != 0) {
        puts("SSDP create socket failed");
        return -1;
    }

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


