#include<iostream>
#include<vector>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<limits>
#include "../include/client.h"

Client::Client(){
    this->socketfd = 0;
    this->port_number = 8012;
    this->ip_address = "127.0.0.1";
    this->isConnected = false;

    //create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        //display error to stderr
        std::cerr << "Error in creating socket" << std::endl;
        exit(1);
    }   
    //initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_addr.sin_port = htons(port_number);
}

void Client::connectToServer(){
    //connect to server
    if(connect(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        //display error to stderr
        std::cerr << "Error in connecting to server" << std::endl;
        return;
    }   
    isConnected = true;
}
bool Client::isConnectedToServer(){
    return isConnected;
}

void Client::sendDataToServer(std::string data, size_t size){
    //send data to server
    if(send(socketfd, data.c_str(), size, 0) == -1){
        //display error to stderr
        std::cerr << "Error in sending data to server" << std::endl;
        exit(1);
    }
}
void Client::receiveDataFromServer(){
    //receive the entire file from server
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if(recv(socketfd, buffer, sizeof(buffer), 0) == -1){
        //display error to stderr
        std::cerr << "Error in receiving data from server" << std::endl;
        exit(1);
    }
    std::cout << buffer << std::endl;

}

bool Client::authenticateUser(std::string username, std::string password){
    //check whether the client is connected to server
    if(!isConnected){
        //display error to stderr
        std::cerr << "Not connected to the server" << std::endl;
        exit(1);
    }
    //authenticate user
    std::string data = "AUTHENTICATE " + username + " " + password;

    //send data to server
    sendDataToServer(data, strlen(data.c_str()));

    //receive data from server
    char buffer[18];
    memset(buffer, 0, sizeof(buffer));
    if(recv(socketfd, buffer, sizeof(buffer), 0) == -1){
        //display error to stderr
        std::cerr << "Error in receiving data from server" << std::endl;
        exit(1);
    }
    //check whether the user is authenticated
    if(std::string(buffer) == "AUTHENTICATED"){
        return true;
    }
    isConnected = false;
    return false;
}

void Client::createUser(std::string username, std::string password){
    //check whether the client is connected to server
    if(!isConnected){
        //display error to stderr
        std::cerr << "Not connected to the server" << std::endl;
        exit(1);
    }
    //create user
    std::string data = "CREATE_USER " + username + " " + password;

    //send data to server
    sendDataToServer(data, strlen(data.c_str()));

    //receive data from server
    char buffer[20];
    memset(buffer, 0, sizeof(buffer));
    if(recv(socketfd, buffer, sizeof(buffer), 0) == -1){
        //display error to stderr
        std::cerr << "Error in receiving data from server" << std::endl;
        exit(1);
    }
    //check whether the user is created
    if(std::string(buffer) == "USER_CREATED"){
        std::cout << "User created successfully" << std::endl;
    }
    else{
        std::cout << "User creation failed" << std::endl;
    }
}

void Client::disconnectClient(){
    //close socket
    sendDataToServer("bye", strlen("bye"));
    close(socketfd);
}