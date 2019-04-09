ODIR = ../obj
IDIR = ../inc
LIBS = -lpthread

CXX = g++
CXXFLAGS = -Wall -I $(IDIR) -std=c++11 
CC = gcc
CFLAGS = -Wall -I $(IDIR)

$(ODIR)/lssdp.o : lssdp.c
	$(CC) $(CFLAGS) -g -c $^ -o $@ 
	
$(ODIR)/ssdp_manager.o: ssdp_manager.cpp
	$(CXX) $(CXXFLAGS) -g -c $^ -o $@
	
main: $(ODIR)/ssdp_manager.o $(ODIR)/lssdp.o main.cpp
	$(CXX) $(CXXFLAGS) -g $^ -o $@ $(LIBS)
	
all: main

clean:
	rm -f main $(ODIR)/*.o
