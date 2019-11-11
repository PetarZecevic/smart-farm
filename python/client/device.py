import socket
import paho.mqtt
import json
import sys
from ssdp_protocol import SSDP_Protocol

def get_device_json(filename):
    dev_info = {}
    dev_state = {}
    with open(filename) as dev_file:
        dev_info = json.load(dev_file)
        for key in dev_info:
            dev_state[key] = {}
            if isinstance(dev_info[key], dict):
                for service_key in dev_info[key]:
                    dev_state[key][service_key] = 'NULL'
            else:
                dev_state[key] = dev_info[key]
    return dev_info, dev_state

if __name__ == '__main__':
    print('IOT Smart Farm Device')
    if len(sys.argv) != 2:
        print('Enter json file that describes the device')
        exit()
    
    dev_info, dev_state = get_device_json(sys.argv[1])
    
    # discovery_service = SSDP_Protocol('M-SEARCH', 'IOTGATEWAY')
    # discovery_service.send_request()
    # response = discovery_service.fetch_response(10)
    # print(response)
