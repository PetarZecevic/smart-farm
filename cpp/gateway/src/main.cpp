#include "ssdp_manager.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

int main()
{
    string name = "Gateway1";
    string userId = "pz97";

    thread ssdp_handler{SSDP_Manager(name, userId, true)};
    ssdp_handler.join();
    
    return 0;
}