#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>	// For sleep
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

using namespace std;

static struct ServerConfiguration_t{
    const string address{"tcp://m16.cloudmqtt.com:15432"};
    const int  qos = 1;
    const chrono::seconds timeout = chrono::seconds(10);
}ServerConfiguration_t;

static struct UserConfiguration_t{
    const string id{"controller1"};
    const string username{"idimoiey"};
    const string password{"DtiBxZxDcQsI"};
}UserConfiguration_t;

class callback : public virtual mqtt::callback
{
public:
    void connected(const string& cause) override
    {
        cout << "\nConnected" << endl;
        if(!cause.empty())
            cout << "\t cause: " << cause << endl;
    }

    void connection_lost(const string& cause) override {
		cout << "\nConnection lost" << endl;
		if (!cause.empty())
			cout << "\tcause: " << cause << endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		cout << "\tDelivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << endl;
	}

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        cout << "\nMessage from server" << endl;
        cout << "Topic: " << msg->get_topic() << endl;
        cout << "Content: " << msg->get_payload_str() << endl;
    }
};

int main(int argc, char** argv)
{
    mqtt::async_client controller(ServerConfiguration_t.address, 
                                  UserConfiguration_t.id);
    callback cb;
    controller.set_callback(cb);

    mqtt::connect_options connection_params;
    connection_params.set_user_name(UserConfiguration_t.username);
    connection_params.set_password(UserConfiguration_t.password);
    
    try{
        // Connect
        cout << "\nConnecting..." << endl;
		mqtt::token_ptr conntok = controller.connect(connection_params);
		cout << "Waiting for the connection..." << endl;
		conntok->wait();
		cout << "  ...OK" << endl;

        // Send message
		cout << "\nSending message..." << endl;
		mqtt::message_ptr pubmsg = mqtt::make_message("farm/temperature", "START");
		pubmsg->set_qos(ServerConfiguration_t.qos);
		controller.publish(pubmsg)->wait_for(ServerConfiguration_t.timeout);
		cout << "  ...OK" << endl;

        // Receive message

        // Disconnect
		cout << "\nDisconnecting..." << endl;
		conntok = controller.disconnect();
		conntok->wait();
		cout << "  ...OK" << endl;

    }catch(const mqtt::exception& exc)
    {
        cerr << exc.what() << endl;
		return 1;
    }
    return 0;
}