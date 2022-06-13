/*
* This class file stores the TCP/IP connection information.
* It also stores the collection of users and blacklisted ips.
* It is used for TCP server program.
*/

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <vector>
#include <user.h>
#define MAX_SIZE 1024

class Server
{
public:
    Server();
    int LoadUsersData();
    int AcceptConnections();
    char* ReceiveDataFromClient(int clientfd);
    int SendDataToClient(int clientfd, const std::string &data);
    bool AuthenticateUser(int clientfd, const User &user);
    int ListDirContents(int clientfd, const std::string &dir);
    int ChangeDir(const std::string &dir, User &user, int clientfd);
    int EditLine(int clientfd, const std::string &file, int linenum);
    int ViewFile(int clientfd, const std::string &file, int start, int end);
    int SelectFile(std::string &filename, const std::string &dir, int clientfd);
private:
    int socketfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    std::vector<User> users;
    char buffer[MAX_SIZE];
};

#endif
