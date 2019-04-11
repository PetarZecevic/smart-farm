ODIR = ../obj
IDIR = ../inc
LDIR = ../lib

CXX = arm-linux-gnueabihf-g++
CXXFLAGS = -Wall -I $(IDIR) -mcpu=cortex-a7 -std=c++11 
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -I $(IDIR) -mcpu=cortex-a7 -mfloat-abi=hard

LIBS = -lMQTTPacketClient

$(ODIR)/lssdp.o : lssdp.c
	$(CC) $(CFLAGS) -g -c $^ -o $@ 
	
$(ODIR)/ssdp_client.o: ssdp_client.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@

# -Wl,-R/libpath eksplicitno naglasava odakle da potrazi biblioteke u runtime linkovanju.
main: $(ODIR)/ssdp_client.o $(ODIR)/lssdp.o main.cpp
	$(CXX) $(CXXFLAGS) -I /usr/local/include -L $(LDIR) -g $^ -o $@ $(LIBS) -Wl,-R$(LDIR)
	
all: main

clean:
	rm -f main $(ODIR)/*.o
