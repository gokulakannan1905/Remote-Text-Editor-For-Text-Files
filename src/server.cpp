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
#define USERS "users.txt"
#define PORT 8012

/*
 * This is the constructor for the server class.
 * It is responsible for initializing the server.
 */
Server::Server()
{
    this->socketfd = 0;
    this->users.clear();
    this->client_addr = {};
    this->client_addr_size = sizeof(this->client_addr);
    memset(this->buffer, 0, MAX_SIZE);
    memset(&server_addr, 0, sizeof(server_addr));    

    /* create socket */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        /* display error to stderr */
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /* initialize server address */
    server_addr = {AF_INET, htons(PORT), INADDR_ANY};

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

int Server::LoadUsersData(){
    /* read data from ../data/users.txt file and store it in users vector */
    std::ifstream file;
    std::string line;   
    std::string users_file = USERS;
    std::string name, password, dir;
    std::stringstream ss;

    file.open(DATA_DIR + users_file);
    if (!file.is_open())
        return -1;
    while (std::getline(file, line))
    {
        ss.str(line);
        ss >> name >> password >> dir;
        if(!name.empty() && !password.empty()){
            User user(name, password);
            users.push_back(user);
        }
        ss.clear();
    }
    file.close();
    return 0;
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
        return -1;
    }
    return client_socketfd;
}

/*
 * This receive function is responsible for receiving the data from the client.
 */
char* Server::ReceiveDataFromClient(int client_socketfd)
{
    /* receive data from client */
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socketfd, buffer, MAX_SIZE, 0) == -1)
    {
        /* display error to stderr */
        perror("recv");
        return NULL;
    }
    return buffer;
}

/*
 * This function is responsible for sending data to the client.
 */
int Server::SendDataToClient(int client_socketfd,const std::string &data)
{
    /* send data to client */
    if (send(client_socketfd, data.c_str(), data.length(), 0) == -1)
    {
        /* display error to stderr */
        perror("send");
        return -1;
    }
    return 0;
}

/*
 * This function is responsible for handling the authentication of the client.
 * It is responsible for checking if the client is authenticated or not.
 * It is responsible for checking if the client is blacklisted or not.
 */
bool Server::AuthenticateUser(int client_socketfd, const User &current_user)
{
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
 * This function is responsible for handling the ls request.
 */
int Server::ListDirContents(int client_socketfd, const std::string &directory)
{
    /* list directory contents */
    DIR *dir;
    std::string buffer, filename;
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
        SendDataToClient(client_socketfd, buffer.c_str());
        closedir(dir);
        return 0;
    }    
    send(client_socketfd, "NO_FILES_FOUND", sizeof("NO_FILES_FOUND"), 0);
    return -1;
}

/*
 * This function is responsible for handling the cd request.
 */
int Server::ChangeDir(const std::string &new_directory, User &current_user, int client_socketfd)
{

    DIR *dir;
    if (new_directory.empty())
    {
        /* change directory to home directory */
        current_user.ChangeDir("");
        /* send switched to home directory message to client */
        send(client_socketfd, "SWITCHED_TO_HOME_DIRECTORY", sizeof("SWITCHED_TO_HOME_DIRECTORY"), 0);
        return 0;
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
                return 0;
            }
            else if (new_directory == ".." || new_directory == "../")
            {
                /* change directory and restrict user to go beyond the ../data/home/username directory */
                if (current_user.GetDir() == "../data/home/" + current_user.GetName())
                {
                    /* send restricted message to client */
                    send(client_socketfd, "DIRECTORY_RESTRICTED", sizeof("DIRECTORY_RESTRICTED"), 0);
                    closedir(dir);
                    return 0;
                }
                /* go to previous directory and remove the last directory from the current directory */
                current_user.ChangeDir(current_user.GetDir().substr(0, current_user.GetDir().find_last_of("/")));
                /* send success message to client */
                send(client_socketfd, "DIRECTORY_CHANGED", sizeof("DIRECTORY_CHANGED"), 0);
                closedir(dir);
                return 0;
            }
        }
        closedir(dir);
    }
    /* send failure message to client */
    send(client_socketfd, "DIRECTORY_NOT_FOUND", sizeof("DIRECTORY_NOT_FOUND"), 0);
    return -1;
}

/*
 * This function is responsible for handling the select request.
 */
int Server::SelectFile(std::string &filename,const std::string &dirname, int client_socketfd)
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
                return 0;
            }
        }
        closedir(dir);
    }
    /* clear filename */
    filename.clear();
    /* send failure message to client */
    send(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"), 0);
    return -1;
}

/*
 * This function is responsible for handling the edit request.
 */
int Server::EditLine(int client_socketfd,const std::string &filename, int line_number)
{
    /* open file in read mode at line_number line */
    std::ifstream file(filename);
    if (!file.is_open())
    {
        /* display error to stderr */
        SendDataToClient(client_socketfd, "FILE_NOT_FOUND");
        return -1;
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
        SendDataToClient(client_socketfd, "INVALID_LINE_NUMBER");
        return -1;
    }

    /* send the selected line with line_number to client */
    line.clear();
    line = std::to_string(line_number) + ":" + lines[line_number - 1];
    SendDataToClient(client_socketfd, line);

    /* receive the edited line from client */
    memset(buffer, 0, sizeof(buffer));
    recv(client_socketfd, buffer, sizeof(buffer), 0);

    //again load the file in a vector
    std::ifstream file1(filename);
    if (!file1.is_open())
    {
        /* display error to stderr */
        SendDataToClient(client_socketfd, "FILE_NOT_FOUND");
        return -1;
    }
    std::string line1;
    lines.clear();
    while (std::getline(file1, line1))
    {
        lines.push_back(line1);
    }   
    /* close file */
    file1.close();
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
    return 0;
}

/*
 * This function is responsible for handling the print request.
 */
int Server::ViewFile(int client_socketfd, const std::string &filename, int start_line, int end_line)
{
    /* open file in read mode */
    std::ifstream file(filename);
    if (!file.is_open())
    {
        /* end failure message to client */
        SendDataToClient(client_socketfd, "FILE_NOT_FOUND");
        return -1;
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
        std::string msg = std::string("INVALID_LINE_NUMBER : Choose between ") + std::to_string(1) + " and " + std::to_string(line_number);
        SendDataToClient(client_socketfd, msg);

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
            SendDataToClient(client_socketfd, line_with_number);
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
    memset(buffer, 0, sizeof(buffer));
    send(client_socketfd, buffer,MAX_SIZE,0);
    return 0;
}
