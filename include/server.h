//avoid multiple inclusions
#ifndef SERVER_H
#define SERVER_H

#include<netinet/in.h>
#include<string>
#include<vector>
#include "./user.h"
class Server{
    int socketfd;
    struct sockaddr_in server_addr,client_addr;
    socklen_t client_addr_size;
    int port_number;
    std::string ip_address;
    std::vector<User> users;
    std::vector<std::string> ip_blacklist;
    public:
    Server();
    int acceptConnections();
    std::string receiveDataFromClient(int);
    void sendDataToClient(int,std::string,size_t);
    bool authenticateUser(int,User);
    void createUser(int,User);

    void listDirContents(int,std::string);
    void changeDir(std::string);
    void editLine(int,std::string,int);
    void viewFile(int,std::string,int,int);
};

#endif