ODIR = ../obj
IDIR = ../inc

CXX = arm-linux-gnueabihf-g++
CXXFLAGS = -Wall -I $(IDIR) -mcpu=cortex-a7 -std=c++11 
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -I $(IDIR) -mcpu=cortex-a7 -mfloat-abi=hard

$(ODIR)/lssdp.o : lssdp.c
	$(CC) $(CFLAGS) -g -c $^ -o $@ 
	
$(ODIR)/ssdp_client.o: ssdp_client.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@
	
main: $(ODIR)/ssdp_client.o $(ODIR)/lssdp.o main.cpp
	$(CXX) $(CXXFLAGS) -g $^ -o $@
	
all: main

clean:
	rm -f main $(ODIR)/*.o
