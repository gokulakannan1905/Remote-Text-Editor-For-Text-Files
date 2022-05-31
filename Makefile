#compiler
CC = g++
#compiler flags
CFLAG = -c -g -Wall
#output file
OUTPUT = -o
#binary file directory
BIN = ./bin
#source file directory
SRC = ./src
#include file directory
INC = ./include
#object file directory
OBJ = ./obj
#include flags
IF = -I $(INC)

#compile all source files

all: build

build: build-server build-client

build-server: $(OBJ)/server.o $(OBJ)/user.o $(SRC)/serverProgram.cpp
	$(CC) $(IF) $(OUTPUT) $(BIN)/server $(OBJ)/server.o $(OBJ)/user.o $(SRC)/serverProgram.cpp

build-client: $(OBJ)/client.o $(SRC)/clientProgram.cpp
	$(CC) $(IF) $(OUTPUT) $(BIN)/client $(OBJ)/client.o $(SRC)/clientProgram.cpp

$(OBJ)/server.o: $(SRC)/server.cpp
	$(CC) $(IF) $(CFLAG) $(OUTPUT) $(OBJ)/server.o $(SRC)/server.cpp
$(OBJ)/client.o: $(SRC)/client.cpp
	$(CC) $(IF) $(CFLAG) $(OUTPUT) $(OBJ)/client.o $(SRC)/client.cpp
$(OBJ)/user.o: $(SRC)/user.cpp
	$(CC) $(IF) $(CFLAG) $(OUTPUT) $(OBJ)/user.o $(SRC)/user.cpp

#run server
server:
	./bin/server
#run client
client:
	./bin/client
clean:
	rm  $(BIN)/server $(BIN)/client $(OBJ)/server.o $(OBJ)/client.o $(OBJ)/user.o