#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>
#include<sstream>
#include "../include/server.h"

int main(){
    //create server object
    char buffer[1024];
    Server server;
    while(1){
    int clientfd = server.acceptConnections();
    pid_t pid = fork();
    if(pid == 0){
    std::string input;
    input = server.receiveDataFromClient(clientfd);
    std::stringstream ss(input);
    std::string cmd,name,passwd;
    ss >> cmd;
    ss >> name;
    ss >> passwd;
    if(server.authenticateUser(clientfd,User(name,passwd)))
        while(1){
            memset(buffer,0,sizeof(buffer));
            recv(clientfd,buffer,sizeof(buffer),0);
            std::cout << buffer << std::endl;
            send(clientfd,buffer,sizeof(buffer),0);
        }
    }
    signal(SIGCHLD,SIG_IGN);
    close(clientfd);
    }
    return 0;
}