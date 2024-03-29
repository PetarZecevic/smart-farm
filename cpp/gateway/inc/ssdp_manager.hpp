#ifndef SSDP_MANAGER_HPP
#define SDDP_MANAGER_HPP

#include <string>
#include <cstdlib>
#include <fstream>
#include "lssdp.h"

class SSDP_Manager
{
public:
    SSDP_Manager(std::string usn, std::string userId, int brokerPort, bool debug);
    SSDP_Manager(const SSDP_Manager& m);
    ~SSDP_Manager();
    bool setInterface();
    bool checkForDevices();
    bool rebindSocket();
    void operator()();
private:
    /*
    Write string to file for log informations.
    Log is defined by template:
        time -> event
        time is hour:min:second.
    */
    void static recordLog(const std::string& logMessage);
    void static log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message);
	int static show_interface_list_and_rebind_socket(lssdp_ctx * lssdp);
	int static show_neighbor_list(lssdp_ctx * lssdp);
	int static show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, std::size_t packet_len);
    lssdp_ctx client_;
    fd_set fs_;
};

#endif // SSDP_MANAGER_HPP