PAHO_C_LIB_DIR ?= /usr/local/lib
PAHO_C_INC_DIR ?= /usr/local/include

CXX = g++
CXXFLAGS = -std=c++11
CPPFLAGS = -I$(PAHO_C_INC_DIR)

LIBS = -L$(PAHO_C_LIB_DIR) -lpaho-mqttpp3 -lpaho-mqtt3a
SRC = controller.cpp
TARGET = controller

all:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)