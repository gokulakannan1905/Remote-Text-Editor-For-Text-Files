/*
 * This is the server class implementation.
 * It is responsible for handling the server side of the program.
 * It is responsible for accepting connections from clients,
 * and for handling the communication between the client and server.
 * It is responsible for handling the authentication of the client.
 * It is responsible for handling the commands sent by the client.
 */

#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits>
#include <dirent.h>
#include <sys/stat.h>
#include <server.h>
#include <user.h>
#define DATA_DIR "../data/"

/*
 * This is the constructor for the server class.
 * It is responsible for initializing the server.
 */
Server::Server()
{
    this->socketfd = 0;
    this->port_number = 8012;
    this->ip_address = "0.0.0.0";
    this->users.clear();
    this->client_addr = {};
    this->client_addr_size = sizeof(this->client_addr);

    /* read data from ../data/users.txt file and store it in users vector */
    std::ifstream file;
    try
    {
        std::string users_file = "users.txt";
        file.open(DATA_DIR + users_file);
        if (!file.is_open())
            /* throw the error if file is not opened */
            throw "File : users.txt not found";
    }
    catch (const char *msg)
    {
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string name, password, dir;
        ss >> name >> password >> dir;
        User user(name, password);
        users.push_back(user);
    }
    file.close();

    /* read data from ../data/ip_blacklist.txt file and store it in ip_blacklist vector */
    try
    {
        std::string ip_blacklist_file = "ip_blacklist.txt";
        file.open(DATA_DIR + ip_blacklist_file);
        /* throw the error if file is not opened */
        if (!file.is_open())
            throw "File : ip_blacklist.txt not found";
    }
    catch (const char *msg)
    {
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    }
    while (std::getline(file, line))
    {
        ip_blacklist.push_back(line);
    }
    file.close();

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

    /* bind socket to server address */
    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        /* display error to stderr */
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* listen for connections */
    if (listen(socketfd, 5) == -1)
    {
        /* display error to stderr */
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

/*
 * destructor for closing the socket and freeing the memory allocated
 */
Server::~Server()
{
    /* close socket */
    close(socketfd);
}

/*
 * This function is responsibe for accepting the client connection.
 */
int Server::AcceptConnections()
{
    /* accept connection from client */
    int client_socketfd = accept(socketfd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_socketfd == -1)
    {
        /* display error to stderr */
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return client_socketfd;
}

/*
 * This receive function is responsible for receiving the data from the client.
 */
std::string Server::ReceiveDataFromClient(int client_socketfd)
{
    /* receive data from client */
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socketfd, buffer, sizeof(buffer), 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        exit(EXIT_FAILURE);
    }
    return std::string(buffer);
}

/*
 * This function is responsible for sending data to the client.
 */
void Server::SendDataToClient(int client_socketfd,const std::string &data, size_t size)
{
    /* send data to client */
    if (send(client_socketfd, data.c_str(), size, 0) == -1)
    {
        /* display error to stderr */
        perror("send");
        exit(EXIT_FAILURE);
    }
}

/*
 * This function is responsible for handling the authentication of the client.
 * It is responsible for checking if the client is authenticated or not.
 * It is responsible for checking if the client is blacklisted or not.
 */
bool Server::AuthenticateUser(int client_socketfd, const User &current_user)
{

    /* get client's ip address */
    std::string client_ip = inet_ntoa(client_addr.sin_addr);

    /* check if client's ip address is in ip_blacklist */
    for (auto ip : ip_blacklist)
    {
        if (client_ip == ip)
        {
            /* send error message to client */
            send(client_socketfd, "error", strlen("error"), 0);

            /* close connection with client */
            close(client_socketfd);
            return false;
        }
    }
    /* check whether user is present in users vector */
    for (auto user : users)
    {
        if (user == current_user)
        {
            /* send success message to client */
            send(client_socketfd, "AUTHENTICATED", strlen("AUTHENTICATED"), 0);
            return true;
        }
    }
    /* send failure message to client */
    send(client_socketfd, "NOT_AUTHENTICATED", strlen("NOT_AUTHENTICATED"), 0);
    return false;
}

/*
 * This function is responsible for creating a new user.
 * This is not in requirements but it is added for testing purpose.
void Server::CreateUser(int client_socketfd, User new_user)
{
    // check whether user is present in users vector
    for (auto user : users)
    {
        if (user == new_user)
        {
            // send failure message to client
            send(client_socketfd, "USER_ALREADY_EXISTS", strlen("USER_ALREADY_EXISTS"), 0);
            return;
        }
    }
    // add user to users vector
    users.push_back(new_user);

    // store the data in ../data/users.txt file
    new_user.StoreData();

    // create user's directory 
    mkdir(new_user.GetDir().c_str(), 0777);

    // send success message to client
    send(client_socketfd, "USER_CREATED", strlen("USER_CREATED"), 0);
}
*/


/*
 * This function is responsible for handling the ls request.
 */
void Server::ListDirContents(int client_socketfd, const std::string &directory)
{
    /* list directory contents */
    DIR *dir;
    std::string buffer, filename;
    buffer.clear();
    if ((dir = opendir(directory.c_str())) != NULL)
    {
    struct dirent *ent;
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_DIR)
            {
                filename.clear();
                /* send the directory contents to client */
                // dont add .. and . to the list
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    filename = "d\t" + std::string(ent->d_name) + "/\n";
                    buffer += filename;                    
                }                
            }
            else if (ent->d_type == DT_REG)
            {
                filename.clear();
                filename = "-\t" + std::string(ent->d_name) + "\n";
                buffer += filename;
            }
        }
        /* remove the last newline character */
        buffer.pop_back();
        send(client_socketfd, buffer.c_str(), buffer.size(), 0);
        closedir(dir);
    }
    else
        send(client_socketfd, "NO_FILES_FOUND", sizeof("NO_FILES_FOUND"), 0);
}

/*
 * This function is responsible for handling the cd request.
 */
void Server::ChangeDir(const std::string &new_directory, User &current_user, int client_socketfd)
{

    DIR *dir;
    if (new_directory.empty())
    {
        /* change directory to home directory */
        current_user.ChangeDir("");
        /* send switched to home directory message to client */
        send(client_socketfd, "SWITCHED_TO_HOME_DIRECTORY", sizeof("SWITCHED_TO_HOME_DIRECTORY"), 0);
        return;
    }
    else if ((dir = opendir((current_user.GetDir()).c_str())) != NULL)
    {
    struct dirent *ent;
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_DIR && ent->d_name == new_directory && new_directory != ".." && new_directory != ".")
            {
                /* change directory to new directory */
                current_user.ChangeDir(current_user.GetDir() + "/" + new_directory);
                /* send success message to client */
                send(client_socketfd, "DIRECTORY_CHANGED", sizeof("DIRECTORY_CHANGED"), 0);
                closedir(dir);
                return;
            }
            else if (new_directory == ".." || new_directory == "../")
            {
                /* change directory and restrict user to go beyond the ../data/home/username directory */
                if (current_user.GetDir() == "../data/home/" + current_user.GetName())
                {
                    /* send restricted message to client */
                    send(client_socketfd, "DIRECTORY_RESTRICTED", sizeof("DIRECTORY_RESTRICTED"), 0);
                    closedir(dir);
                    return;
                }
                /* go to previous directory and remove the last directory from the current directory */
                current_user.ChangeDir(current_user.GetDir().substr(0, current_user.GetDir().find_last_of("/")));
                /* send success message to client */
                send(client_socketfd, "DIRECTORY_CHANGED", sizeof("DIRECTORY_CHANGED"), 0);
                closedir(dir);
                return;
            }
        }
        closedir(dir);
    }
    /* send failure message to client */
    send(client_socketfd, "DIRECTORY_NOT_FOUND", sizeof("DIRECTORY_NOT_FOUND"), 0);
}

/*
 * This function is responsible for handling the select request.
 */
void Server::SelectFile(std::string &filename,const std::string &dirname, int client_socketfd)
{
    /* check whether file exists in current directory */
    DIR *dir;
    if ((dir = opendir(dirname.c_str())) != NULL)
    {
    struct dirent *ent;
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG && std::string(ent->d_name) == filename)
            {
                /* set the filename to fully qualified path */
                filename = dirname + "/" + filename;
                /* send success message to client */
                send(client_socketfd, "FILE_SELECTED", sizeof("FILE_SELECTED"), 0);
                closedir(dir);
                return;
            }
        }
        closedir(dir);
    }
    /* clear filename */
    filename.clear();
    /* send failure message to client */
    send(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"), 0);
}

/*
 * This function is responsible for handling the edit request.
 */
void Server::EditLine(int client_socketfd,const std::string &filename, int line_number)
{
    /* open file in read mode at line_number line */
    std::ifstream file(filename);
    if (!file.is_open())
    {
        /* display error to stderr */
        SendDataToClient(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"));
        return;
    }
    /* store the file in a vector */
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    /* close file */
    file.close();
    /* check whether line_number is valid */
    if (line_number > (int)lines.size())
    {
        /* send failure message to client */
        SendDataToClient(client_socketfd, "INVALID_LINE_NUMBER", sizeof("INVALID_LINE_NUMBER"));
        return;
    }

    /* send the selected line with line_number to client */
    line.clear();
    line = std::to_string(line_number) + ":" + lines[line_number - 1];
    SendDataToClient(client_socketfd, line, line.length());

    /* receive the edited line from client */
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socketfd, buffer, sizeof(buffer), 0);
    /* replace the line in the vector */
    if(strcmp(buffer,"0")!=0)
    lines[line_number - 1] = buffer;
    /* open file in write mode */
    std::ofstream file_write(filename);
    /* write the vector to file */
    for (auto single_line : lines)
    {
        file_write << single_line << std::endl;
    }
    /* close file */
    file_write.close();
}

/*
 * This function is responsible for handling the print request.
 */
void Server::ViewFile(int client_socketfd, const std::string &filename, int start_line, int end_line)
{
    /* open file in read mode */
    std::ifstream file(filename);
    if (!file.is_open())
    {
        /* end failure message to client */
        SendDataToClient(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"));
        return;
    }

    //get the number of lines in the file
    std::string line;
    int line_number = 0;
    std::ifstream file_read(filename);
    while (std::getline(file_read, line))
    {
        line_number++;
    }
    file_read.close();
    
    /* check whether start_line and end_line is valid */
    if(end_line > line_number || start_line > line_number )
    {
        //send the failure msg with start and end line of the file to client
        std::string msg = std::string("INVALID_LINE_NUMBER Choose between ") + std::to_string(1) + " and " + std::to_string(line_number);
        SendDataToClient(client_socketfd, msg, msg.length());

    }else{

    /* read file contents and send it to the client */
    line.clear();
    int i = 1;
    while (std::getline(file, line))
    {
        if (i >= start_line)
        {
            /* send line with line number to client */
            std::string line_with_number = std::to_string(i) + " " + line + "\n";
            send(client_socketfd, line_with_number.c_str(), strlen(line_with_number.c_str()), 0);
            if (end_line != -1 && i == end_line)
            {
                file.close();
                break;
            }
        }
        i++;
    }
    file.close();
    }
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    write(client_socketfd, buff, 1024);
}
