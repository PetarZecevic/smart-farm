INC = inc
OBJ = obj

CXX = arm-linux-gnueabihf-g++
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -I $(INC) -mcpu=cortex-a7

SRC = src/client.cpp
TARGET = client

lssdp:
	$(CC) $(CFLAGS) -mfloat-abi=hard -c -o $(OBJ)/lssdp.o src/lssdp.c
	
client_release:
	$(CXX) $(CFLAGS) -O3 -fPIC -c $(SRC) -o $(OBJ)/$@.o 
	$(CXX) $(OBJ)/$@.o $(OBJ)/lssdp.o -o $(TARGET)

client_debug:
	$(CXX) $(CFLAGS) -g3 -c -fPIC -o $(OBJ)/$@.o $(OBJ)/lssdp.o $(SRC)
	$(CXX) -o $(TARGET) $@.o
	
clean:
	rm -f $(OBJ)/*.o $(TARGET)
