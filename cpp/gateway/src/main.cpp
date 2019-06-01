#include "ssdp_manager.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>
#include "mqtt_manager.hpp"

using namespace std;

int main()
{
    string name = "Gateway1";
    string userId = "pz97";
    string brokerAddr = "tcp://localhost:1883";

    mqtt::connect_options con_opts;
    con_opts.set_connect_timeout(1);
    con_opts.set_user_name(name);
    con_opts.set_mqtt_version(3);
    con_opts.set_clean_session(true);

    MQTT_Manager mqtt_manager(userId, name, brokerAddr);
    if(mqtt_manager.connectToBroker(con_opts))
    {
        if(mqtt_manager.start())
        {
            std::cout << "Gateway started" << std::endl;
            thread ssdp_handler{SSDP_Manager(name, userId, 1883, true)};
            ssdp_handler.join();
        }
    }
    return 0;
}
