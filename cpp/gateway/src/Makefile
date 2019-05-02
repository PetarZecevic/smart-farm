ODIR = ../obj
IDIR = ../inc
LDIR = ../lib

LIBS = -lssdp -lpthread -lpaho-mqttpp3 -lpaho-mqtt3a

CXX = g++
CXXFLAGS = -Wall -I $(IDIR) -L $(LDIR) -std=c++11 

$(ODIR)/ssdp_manager.o: ssdp_manager.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

$(ODIR)/mqtt_manager.o: mqtt_manager.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

main: $(ODIR)/mqtt_manager.o $(ODIR)/ssdp_manager.o main.cpp
	$(CXX) $(CXXFLAGS) -g $^ -o $@ $(LIBS)
	
all: main

clean:
	rm -f main $(ODIR)/*.o
