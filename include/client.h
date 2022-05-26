//avoid multiple inclusions
#ifndef CLIENT_H
#define CLIENT_H

#include<netinet/in.h>
#include<string>
class Client{
    int socketfd;
    struct sockaddr_in server_addr;
    int port_number;
    std::string ip_address;
    std::string username;
    unsigned long password;
    bool isConnected;
    public:
    Client();
    void connectToServer();
    bool authenticateUser(std::string, std::string);
    void sendDataToServer(std::string,size_t);
    void receiveDataFromServer();
    void createUser(std::string, std::string);
    void disconnectClient();
};

#endif