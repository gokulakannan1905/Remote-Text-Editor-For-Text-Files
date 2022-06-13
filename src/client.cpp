/*
 * This file has the implementation of the client class member functions.
 */

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sstream>
#include <limits>
#include <client.h>
#define PORT 8012
#define IP "127.0.0.1"
/*
* TCP client class constructor for creating socket and initializing variables
*/
Client::Client()
{
    // Initializing the client variables
    this->socketfd = 0;
    this->isConnected = false;
    memset(this->buffer, 0, MAX_SIZE);
    memset(&server_addr, 0, sizeof(server_addr)); 
}

int Client::CreateSocket(){
    /* create socket */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        /* display error to stderr */
        perror("socket");
        return -1;
    }
    /* initialize server address */
    server_addr = {AF_INET, htons(PORT), inet_addr(IP)};
    return socketfd;
}


/*
 * This is a getter function for getting the socket descriptor
 */
int Client::GetSocketfd()
{
    if(socketfd>0)
    return this->socketfd;
    return -1;  // return -1 if socketfd is not initialized
}


/*
 * This function is used to connect to the server
 */
int Client::ConnectToServer()
{
    /* connect to server */
    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        /* display error to stderr */
        perror("connect");
        return -1;
    }
    isConnected = true;
    return 0;
}


/*
 * This function send data to the server
 */
int Client::SendDataToServer(const std::string &data)
{
    /* send data to server */
    
    if ( !data.empty() && send(socketfd, data.c_str(), data.length(), 0) == -1)
    {
        /* display error to stderr */
        perror("send");
        return -1;
    }
    return 0;
}


/*
 * This function is used to receive data from the server and display it
 */
char* Client::ReceiveDataFromServer()
{
    memset(buffer, 0, sizeof(buffer));
    if (recv(socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        exit(EXIT_FAILURE);
    }
    /* display data to stdout */
    std::cout << buffer << std::endl;
    return buffer;
}


/*
 * This function is used to authenticate the user
 */
bool Client::AuthenticateUser(const std::string &username,const std::string &password)
{
    /* check whether the client is connected to server */
    if (!isConnected)
    {
        /* display error to stderr */
        std::cerr << "Not connected to the server" << std::endl;
        exit(EXIT_FAILURE);
    }
    /* authenticate user */
    std::string data = "AUTHENTICATE " + username + " " + password;

    /* send data to server */
    if(SendDataToServer(data)==-1){
        std::cerr << "Error in sending data to server" << std::endl;
    }

    /* receive data from server */
    memset(buffer, 0, sizeof(buffer));
    if (recv(socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    
    /* check whether the user is authenticated */
    if (std::string(buffer) == "AUTHENTICATED")
    return true;
    isConnected = false;
    return false;
}

int Client::EditLine(){
    
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = read(socketfd, buffer, sizeof(buffer));
    if (strcmp("0", buffer) == 0)
    {
        std::cerr << "FILE_NOT_SELECTED: use select <FILENAME> command" << std::endl;
        return -1;
    }
    if (strcmp("INVALID_LINE_NUMBER", buffer) == 0)
    {
        std::cerr << "INVALID_LINE_NUMBER" << std::endl;
        return -1;
    }
    std::cout << buffer << std::endl;

    /* ask user to edit the line received from server and send it to server */
    std::string edited_line;
    std::cout << "Enter changes to the line: ";
    std::getline(std::cin, edited_line);
    if (edited_line.length() == 0)
    SendDataToServer("0");
    else
    SendDataToServer(edited_line);
    return 0;
}


/* 
 * This function disconnects the client from the server
 */
int Client::DisconnectClient()
{
    /* send "bye" to server and close the socket */
    SendDataToServer("bye");
    close(socketfd);
    isConnected = false;
    return 0;
}


/*
 * This function display the contents of the file in the server
 */ 
int Client::ReceiveFile()
{
    bool isNotEnd = true;
    while (isNotEnd)
    {
        int len = 0;
        /* receive data from server */
        memset(buffer, 0, sizeof(buffer));
        len = recv(socketfd, buffer, sizeof(buffer), 0);
        if (strcmp("0", buffer) == 0)
        {
            std::cerr << "FILE_NOT_SELECTED : use select <FILENAME> command" << std::endl;
            return -1;
        }
        isNotEnd = buffer[0];
        write(1, buffer, len);
    }
    std::cout << std::endl;
    return 0;
}
