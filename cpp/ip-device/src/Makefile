ODIR = ../obj
IDIR = ../inc
LDIR = ../lib

CXX = arm-linux-gnueabihf-g++
CXXFLAGS = -Wall -I $(IDIR) -mcpu=cortex-a7 -std=c++11

LIBS = -lssdp -lMQTTPacketClient

$(ODIR)/ssdp_client.o: ssdp_client.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

$(ODIR)/ipinfo.o : ipinfo.cpp
	$(CXX) $(CXXFLAGS) -D RAPIDJSON_HAS_STDSTRING -I /usr/local/include -g -c $^ -o $@

$(ODIR)/mqtt_client.o : mqtt_client.cpp
	$(CXX) $(CXXFLAGS) -I /usr/local/include -g -c $^ -o $@

# -Wl,-R/libpath eksplicitno naglasava odakle da potrazi biblioteke u runtime linkovanju.
main: $(ODIR)/ssdp_client.o $(ODIR)/mqtt_client.o $(ODIR)/ipinfo.o main.cpp
	$(CXX) $(CXXFLAGS) -I /usr/local/include -L $(LDIR) -g $^ -o $@ $(LIBS) -Wl,-R$(LDIR)

bulb: $(ODIR)/ssdp_client.o $(ODIR)/mqtt_client.o $(ODIR)/ipinfo.o bulb.cpp
	$(CXX) $(CXXFLAGS) -I /usr/local/include -L $(LDIR) -g $^ -o $@ $(LIBS) -lwiringPi -Wl,-R$(LDIR)
	
all: main

clean:
	rm -f main bulb $(ODIR)/*.o
