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
public:
    Client();
    int CreateSocket();
    int GetSocketfd();
    int ConnectToServer();
    bool AuthenticateUser(const std::string &username, const std::string &password);
    int SendDataToServer(const std::string &data);
    char* ReceiveDataFromServer();
    int DisconnectClient();
    int EditLine();
    int ReceiveFile();
private:
    int socketfd;
    struct sockaddr_in server_addr;
    bool isConnected;
    char buffer[MAX_SIZE];
};

#endif
