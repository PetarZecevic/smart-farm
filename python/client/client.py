# Client application
import paho.mqtt.client as mqtt
import os
import time

# MQTT Broker info
broker = "m16.cloudmqtt.com"
port = 15432
ssl_port = 25432
username = "idimoiey" 
password = "DtiBxZxDcQsI"

def on_message(client, userdata, message):
	message_content = message.payload.decode('utf-8')
	if message.topic == 'farm/water_system':
		if message_content == 'WARNING':
			print('Water supply needed')
		else:
			print('Water fine')
	elif message.topic == 'farm/light_system':
		print('Light system: ' + message_content)

if __name__ == "__main__":
	
	connection_error = 0
	
	client = mqtt.Client("smartFarmClient")
	client.on_message=on_message
	client.username_pw_set(username, password)
	client.connect(broker, port)
	
	client.subscribe("farm/water_system")
	client.subscribe('farm/light_system')
	
	print("Ready for interaction")

	while connection_error == 0:
		# Processing events from broker.
		connection_error = client.loop()		
		time.sleep(1)
