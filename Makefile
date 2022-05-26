#compiler
CC = g++
#compiler flags
CFLAG = -c
#output file
OUTPUT = -o
#binary file directory
BIN = ./bin
#source file directory
SRC = ./src
#include file directory
INCLUDE = ./include
#object file directory
OBJ = ./obj

#compile all source files

all: build

build: $(OBJ)/server.o $(OBJ)/client.o $(OBJ)/user.o
	$(CC) $(OUTPUT) $(BIN)/server $(OBJ)/server.o $(OBJ)/user.o $(SRC)/serverProgram.cpp
	$(CC) $(OUTPUT) $(BIN)/client $(OBJ)/client.o $(SRC)/clientProgram.cpp

$(OBJ)/server.o: $(SRC)/server.cpp
	$(CC) $(CFLAG) $(OUTPUT) $(OBJ)/server.o $(SRC)/server.cpp
$(OBJ)/client.o: $(SRC)/client.cpp
	$(CC) $(CFLAG) $(OUTPUT) $(OBJ)/client.o $(SRC)/client.cpp
$(OBJ)/user.o: $(SRC)/user.cpp
	$(CC) $(CFLAG) $(OUTPUT) $(OBJ)/user.o $(SRC)/user.cpp

#run server
server:
	./bin/server
#run client
client:
	./bin/client
