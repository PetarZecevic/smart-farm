ODIR = ../obj
IDIR = ../inc
LDIR = ../lib

LIBS = -lssdp -lpthread -lpaho-mqttpp3 -lpaho-mqtt3a

CXX = g++
CXXFLAGS = -Wall -I $(IDIR) -L $(LDIR) -std=c++11

$(ODIR)/ssdp_client.o: ssdp_client.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

$(ODIR)/mqtt_client.o: mqtt_client.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

$(ODIR)/parser.o : parser.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

main: $(ODIR)/parser.o $(ODIR)/mqtt_client.o $(ODIR)/ssdp_client.o main.cpp
	$(CXX) $(CXXFLAGS) -g $^ -o $@ $(LIBS)

all: main

clean:
	rm -f main $(ODIR)/*.o
