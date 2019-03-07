import RPi.GPIO as GPIO
import time

class DistanceSensor():
	def __init__(self):
		self.trigger_pin = 17
		self.echo_pin = 4
		GPIO.setup(self.trigger_pin, GPIO.OUT)
		GPIO.setup(self.echo_pin, GPIO.IN)

	def measureDistance(self):
		# set Trigger to HIGH
		GPIO.output(self.trigger_pin, True)
	 
		# set Trigger after 0.01ms to LOW
		time.sleep(0.00001)
		GPIO.output(self.trigger_pin, False)
	 
		StartTime = time.time()
		StopTime = time.time()
	 
		# save StartTime
		while GPIO.input(self.echo_pin) == 0:
			StartTime = time.time()
	 
		# save time of arrival
		while GPIO.input(self.echo_pin) == 1:
			StopTime = time.time()
	 
		# time difference between start and arrival
		TimeElapsed = StopTime - StartTime
		# multiply with the sonic speed (34300 cm/s)
		# and divide by 2, because there and back
		distance = (TimeElapsed * 34300) / 2
		
		return distance
