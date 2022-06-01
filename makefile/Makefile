#compiler
CC = g++
#compiler flags 
CFLAG = -g -Wall
#output file flag
OUTPUT = -o
#binary file directory
BIN = ../bin
#source file directory
SRC = ../src
#include file directory
INC = ../include
#object file directory
OBJ = ../obj
#include flag
IF = -I $(INC)

#compile all source files

all: build

build: build-server build-client

build-server: $(OBJ)/server.o $(OBJ)/user.o $(SRC)/serverProgram.cpp
	$(CC) $(IF) $(CFLAG) $(OUTPUT) $(BIN)/server $(OBJ)/server.o $(OBJ)/user.o $(SRC)/serverProgram.cpp

build-client: $(OBJ)/client.o $(SRC)/clientProgram.cpp
	$(CC) $(IF) $(CFLAG) $(OUTPUT) $(BIN)/client $(OBJ)/client.o $(SRC)/clientProgram.cpp

$(OBJ)/server.o: $(SRC)/server.cpp
	$(CC) $(IF) -c $(OUTPUT) $(OBJ)/server.o $(SRC)/server.cpp

$(OBJ)/client.o: $(SRC)/client.cpp
	$(CC) $(IF) -c $(OUTPUT) $(OBJ)/client.o $(SRC)/client.cpp

$(OBJ)/user.o: $(SRC)/user.cpp
	$(CC) $(IF) -c $(OUTPUT) $(OBJ)/user.o $(SRC)/user.cpp

#run server
server:
	$(BIN)/server

#run client
client:
	$(BIN)/client

clean:
	rm  $(BIN)/server $(BIN)/client $(OBJ)/server.o $(OBJ)/client.o $(OBJ)/user.o