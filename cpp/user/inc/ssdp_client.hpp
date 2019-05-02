#ifndef SSDPCLIENT_HPP
#define SSDPCLIENT_HPP

#include <cstdlib>
#include <string>
#include "lssdp.h"

class SSDP_Client
{
public:
	SSDP_Client(std::string usn, bool debug);
	bool checkMessages();
	bool searchControler();
	bool rebindSocket();
	void findGateway();
	std::string getLogTopic() { return gatewayLogTopic_; };
	std::string getLocation() { return gatewayLocation_; };
	std::string getUSN() { return usn_; };
private:
	void static log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message);
	int static show_interface_list_and_rebind_socket(lssdp_ctx * lssdp);
	int static show_neighbor_list(lssdp_ctx * lssdp);
	int static show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, std::size_t packet_len);
	lssdp_ctx client_; // structure that describes all fields in protocol.
	fd_set fs_; // used for asynchronius I/O.
	std::string usn_;
	std::string gatewayLogTopic_;
	std::string gatewayLocation_;
};

#endif // SSDPCLIENT_HPP
