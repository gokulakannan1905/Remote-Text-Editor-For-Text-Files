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

/*
* TCP client class constructor for creating socket and initializing variables
*/
Client::Client()
{
    // Initializing the client variables
    this->socketfd = 0;
    this->port_number = 8788;
    this->ip_address = "127.0.0.1";
    this->isConnected = false;
    this->server_addr = {};

    /* create socket */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        /* display error to stderr */
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* initialize server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_addr.sin_port = htons(port_number);
}


/*
 * destructor for closing the socket and freeing the memory allocated
 */
Client::~Client()
{
    /* close socket */
    close(socketfd);
}


/*
 * This is a getter function for getting the socket descriptor
 */
int Client::GetSocketfd()
{
    return this->socketfd;
}


/*
 * This function is used to connect to the server
 */
void Client::ConnectToServer()
{
    /* connect to server */
    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        /* display error to stderr */
        perror("connect");
        exit(EXIT_FAILURE);
    }
    isConnected = true;
}


/*
 * This function send data to the server
 */
void Client::SendDataToServer(const std::string &data, size_t size)
{
    /* send data to server */
    if (send(socketfd, data.c_str(), size, 0) == -1)
    {
        /* display error to stderr */
        perror("send");
        exit(EXIT_FAILURE);
    }
}


/*
 * This function is used to receive data from the server and display it
 */
void Client::ReceiveDataFromServer()
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (recv(socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        exit(EXIT_FAILURE);
    }
    /* display data to stdout */
    std::cout << buffer << std::endl;
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
    SendDataToServer(data, data.length());

    /* receive data from server */
    char buffer[18];
    memset(buffer, 0, sizeof(buffer));
    if (recv(socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        exit(EXIT_FAILURE);
    }
    /* check whether the user is authenticated */
    if (std::string(buffer) == "AUTHENTICATED")
    {
        return true;
    }
    isConnected = false;
    return false;
}


/*
 * This function is used to create a new user
 */
void Client::CreateUser(const std::string &username,std::string &password)
{
    /* check whether the client is connected to server */
    if (!isConnected)
    {
        /* display error to stderr */
        std::cerr << "Not connected to the server" << std::endl;
        exit(EXIT_FAILURE);
    }
    /* hash the password using std::hash algorithm and convert it to string */
    std::hash<std::string> hash_fn;
    std::stringstream ss;
    ss << hash_fn(password);
    ss >> password;

    /* concatenate the data to be sent to server */
    std::string data = "create " + username + " " + password;

    /* send data to server */
    SendDataToServer(data, data.length());

    /* receive data from server */
    char buffer[20];
    memset(buffer, 0, sizeof(buffer));
    if (recv(socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        exit(EXIT_FAILURE);
    }
    /* check whether the user is created */
    if (std::string(buffer) == "USER_CREATED")
        std::cout << "USER_CREATED_SUCCESSFULLY" << std::endl;
    else
        std::cerr << "FAILED_TO_CREATE_USER" << std::endl;
}

void Client::EditLine(){
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = read(socketfd, buffer, sizeof(buffer));
    if (strcmp("0", buffer) == 0)
    {
        std::cerr << "FILE_NOT_SELECTED: use select <FILENAME> command" << std::endl;
        return;
    }
    if (strcmp("INVALID_LINE_NUMBER", buffer) == 0)
    {
        std::cerr << "INVALID_LINE_NUMBER" << std::endl;
        return;
    }
    write(1, buffer, bytes_read);

    /* ask user to edit the line received from server and send it to server */
    std::string edited_line;
    std::cout << "\nEnter changes to the line: ";
    /* allow user to type empty line also */
    std::getline(std::cin, edited_line);
    if (edited_line.length() == 0)
        SendDataToServer("0", 1);
    else
     SendDataToServer(edited_line, edited_line.length());
}


/* 
 * This function disconnects the client from the server
 */
void Client::DisconnectClient()
{
    /* send "bye" to server and close the socket */
    SendDataToServer("bye", strlen("bye"));
    close(socketfd);
}


/*
 * This function display the contents of the file in the server
 */ 
void Client::ReceiveFile()
{
    bool isNotEnd = true;
    char buffer[MAX_SIZE];
    while (isNotEnd)
    {
        int len = 0;
        /* receive data from server */
        memset(buffer, 0, sizeof(buffer));
        len = recv(socketfd, buffer, sizeof(buffer), 0);
        if (strcmp("0", buffer) == 0)
        {
            std::cerr << "FILE_NOT_SELECTED : use select <FILENAME> command" << std::endl;
            return;
        }
        isNotEnd = buffer[0];
        write(1, buffer, len);
    }
    std::cout << "\nFile received successfully" << std::endl;
}
