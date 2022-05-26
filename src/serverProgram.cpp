#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include<sys/types.h>
#include<unistd.h>
#include<sstream>
#include "../include/server.h"

int main(){
    //create server object
    char buffer[1024];
    Server server;
    int clientfd = server.acceptConnections();
    std::string input;
    input = server.receiveDataFromClient(clientfd);
    std::stringstream ss(input);
    std::string cmd,name,passwd;
    ss >> cmd;
    ss >> name;
    ss >> passwd;
    if(server.authenticateUser(clientfd,User(name,passwd)))
    memset(buffer,0,sizeof(buffer));
    recv(clientfd,buffer,sizeof(buffer),0);
    std::cout << buffer << std::endl;
    return 0;
}