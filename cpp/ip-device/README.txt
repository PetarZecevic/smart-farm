Executables are gonna be executed on raspberry pi 2B, cross-compile is done in
Makefile.

Prerequisities:
	Eclipse Paho Embedded MQTT C++ library installed and .h files moved to /usr/local/include/mqtt-embedded directory,
	.so files located in lib folder, .so files built for raspberry.
	
	Rapid JSON lib:
		.h files moved to /usr/local/include/rapidjson directory

Run program as root, because pwm functionality from wiringPi requires that.
