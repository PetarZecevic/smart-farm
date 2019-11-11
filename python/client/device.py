import socket
import paho.mqtt.client as mqttclient
import json
import sys
import time
from ssdp_protocol import SSDP_Protocol

device_info = {}
device_state = {}

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

def authorization_callback(client, user_data, message):
    content = message.payload.decode('utf-8')
    if content == 'OK':
        print('Authentication succeded')
    else:
        print('Authentication failed')
        client.disconnect()

def update_callback(client, user_data, message):
    global device_state
    content = message.payload.decode('utf-8')
    print(content)

def get_callback(client, user_data, message):
    global device_state
    content = message.payload.decode('utf-8')
    print(content)

if __name__ == '__main__':
    print('IOT Smart Farm Device')
    if len(sys.argv) != 2:
        print('Enter json file that describes the device')
        exit()
    
    dev_info, dev_state = get_device_json(sys.argv[1])

    discovery_service = SSDP_Protocol('M-SEARCH', 'IOTGATEWAY')
    discovery_service.send_request()
    response = discovery_service.fetch_response(10)

    print('Found gateway, start mqtt')

    broker = response['LOCATION'].split(':', maxsplit=1)[0]
    broker_port = response['LOCATION'].split(':', maxsplit=1)[1]
    gateway_owner_id = response['SM_ID'].split('/')[0]
    topic_template = \
        gateway_owner_id + '/' + \
        dev_info['group'] + '/' + \
        dev_info['id'] + '/' + \
        '{0}'

    mqtt_client = mqttclient.Client(dev_info['id'])
    mqtt_client.connect(broker, int(broker_port))
    
    mqtt_client.message_callback_add(topic_template.format('log'), authorization_callback)
    mqtt_client.message_callback_add(topic_template.format('update'),update_callback)
    mqtt_client.message_callback_add(topic_template.format('get'),get_callback)

    mqtt_client.subscribe([
        (topic_template.format('log'),2),
        (topic_template.format('update'),1),
        (topic_template.format('get'),1)])
    
    mqtt_client.publish(response['SM_ID'], json.dumps(dev_info), qos=2, retain=False)
    
    while True:
        status = mqtt_client.loop()
        if status != 0:
            break
