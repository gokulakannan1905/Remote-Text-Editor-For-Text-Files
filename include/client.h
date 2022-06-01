/*
* This class is used to store the TCP/IP connection information.
* It is used for TCP client program.
*/

#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#define MAX_SIZE 1024

class Client
{
private:
    int socketfd;
    struct sockaddr_in server_addr;
    int port_number;
    std::string ip_address;
    bool isConnected;

public:
    Client();
    int GetSocketfd();
    void ConnectToServer();
    bool AuthenticateUser(const std::string &username, const std::string &password);
    void SendDataToServer(const std::string &data, size_t size);
    void ReceiveDataFromServer();
    void CreateUser(const std::string &username,std::string &password);
    void DisconnectClient();
    void ReceiveFile();
    ~Client();
};

#endif
