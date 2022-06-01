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

class Server
{
private:
    int socketfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    int port_number;
    std::string ip_address;
    std::vector<User> users;
    std::vector<std::string> ip_blacklist;

public:
    Server();
    int AcceptConnections();
    std::string ReceiveDataFromClient(int);
    void SendDataToClient(int clientfd, std::string data, size_t size);
    bool AuthenticateUser(int clientfd, const User &user);
    void CreateUser(int clientfd, User user);
    void ListDirContents(int clientfd, std::string dir);
    void ChangeDir(std::string dir, User &user, int clientfd);
    void EditLine(int clientfd, std::string file, int linenum);
    void ViewFile(int clientfd, std::string file, int start_line_no, int end_line_no);
    void SelectFile(std::string &filename, std::string dir, int clientfd);
    ~Server();
};

#endif
