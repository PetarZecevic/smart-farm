import paho.mqtt.client as mqtt
import os
import time
from random import randint
#mport RPi.GPIO as GPIO
#from distance_sensor import DistanceSensor
#from light import LightSensor

#GPIO.setmode(GPIO.BCM)

# MQTT Broker info
broker = "m16.cloudmqtt.com"
port = 15432
ssl_port = 25432
username = "idimoiey" 
password = "DtiBxZxDcQsI"

#water_level_sensor = DistanceSensor();
water_level_threshold = 15 #cm
water_level = 0
sent_notification = True

#light_sensor = LightSensor()
turned_lights = False

def on_message(client, userdata, message):
	message_content = message.payload.decode('utf-8')
	if message.topic == 'farm/security/gate':
		if message_content == 'OPEN':
			print('Open gate to verified guest')
			time.sleep(1)
			print('Guest entered')
			print('Close gate')
			time.sleep(1)

def read_water(mqtt_client):
	global water_level, water_level_threshold, sent_notification
	#water_level = water_level_sensor.measureDistance()
	water_level = randint(0, 15)
	if water_level > water_level_threshold and not sent_notification:
		mqtt_client.publish('farm/water_system', 'WARNING')
		sent_notification = True
		print("Critical water level: " + str(water_level))
		time.sleep(1)
	else:
		sent_notification = False
		
def read_light(mqtt_client):
	global turned_lights
	#light_level = light_sensor.measureLightIntensity()
	light_level = randint(0, 1023)
	if light_level < 300 and not turned_lights:
		print('Turn on lights')
		mqtt_client.publish('farm/light_system', 'ON')
		turned_lights = True
		time.sleep(1)
	elif light_level > 600 and turned_lights:
		print('Turn off lights')
		mqtt_client.publish('farm/light_system', 'OFF')
		turned_lights = False
		time.sleep(1)
	
if __name__ == '__main__':

	client = mqtt.Client("smartFarmController")
	client.on_message = on_message
	
	client.username_pw_set(username, password)
	client.connect(broker, port)		
	
	client.publish('farm/water_system')
	client.publish('farm/light_system')
	client.subscribe('farm/security/gate')
	
	time.sleep(2)
	print('System ready')

	error_code = 0

	while error_code == 0:
		error_code = client.loop()
		read_water(client)
		read_light(client)