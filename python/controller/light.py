import RPi.GPIO as GPIO
import time

class LightSensor():
	def __init__(self):
		self.light_pin = 23
		GPIO.setup(self.light_pin, GPIO.IN)
		
	def measureLightIntensity(self):
		return GPIO.input(self.light_pin)
		
if __name__ == '__main__':
	GPIO.setmode(GPIO.BCM)
	light_sen = LightSensor()
	while True:
		print(light_sen.measureLightIntensity())
		time.sleep(1)
