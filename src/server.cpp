#include<iostream>
#include<fcntl.h>
#include<string.h>
#include<sstream>
#include<vector>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<limits>
#include<dirent.h>
#include<sys/stat.h>
#include "../include/server.h"
#include "../include/user.h"

Server::Server(){
    this->socketfd = 0;
    this->port_number = 8888;
    this->ip_address = "0.0.0.0";
    this->users.clear();

    //read data from ../data/users.txt file and store it in users vector
    std::ifstream file;
    file.open("../data/users.txt");
    //handle the error if file is not opened
    if(!file.is_open()){
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    std::string line;
    while(std::getline(file, line)){
        std::stringstream ss(line);
        std::string name, password, dir;
        ss >> name >> password >> dir;
        User user(name,password);
        users.push_back(user);
    }
    file.close();

    //read data from ../data/ip_blacklist.txt file and store it in ip_blacklist vector
    file.open("../data/ip_blacklist.txt");
    //handle the error if file is not opened
    if(!file.is_open()){
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    while(std::getline(file, line)){
        ip_blacklist.push_back(line);
    }
    file.close();

    //create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        //display error to stderr
        std::cerr << "Error in creating socket" << std::endl;
        exit(1);
    }
    //initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_addr.sin_port = htons(port_number);

    //bind socket to server address
    if(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        //display error to stderr
        std::cerr << "Error in binding socket to server address" << std::endl;
        exit(1);
    }

    //listen for connections
    if(listen(socketfd, 5) == -1){
        //display error to stderr
        std::cerr << "Error in listening for connections" << std::endl;
        exit(1);
    }
    std::cout << "Server is listening on port "<< port_number << std::endl;
}

int Server::acceptConnections(){
    //accept connection from client
    int client_socketfd = accept(socketfd, (struct sockaddr*)&client_addr, &client_addr_size);
    if(client_socketfd == -1){
        //display error to stderr
        std::cerr << "Error in accepting connection from client" << std::endl;
        exit(1);
    }
    return client_socketfd;
}


std::string Server::receiveDataFromClient(int client_socketfd){
    //receive data from client
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if(recv(client_socketfd, buffer, sizeof(buffer), 0) == -1){
        //display error to stderr
        std::cerr << "Error in receiving data from client" << std::endl;
        exit(1);
    }
    return std::string(buffer);
}

void Server::sendDataToClient(int client_socketfd, std::string data, size_t size){
    //send data to client
    if(send(client_socketfd, data.c_str(), size, 0) == -1){
        //display error to stderr
        std::cerr << "Error in sending data to client" << std::endl;
    }
}

bool Server::authenticateUser(int client_socketfd,User current_user){
    //get client's ip address
    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    //check if client's ip address is in ip_blacklist
    for(auto ip : ip_blacklist){
        if(client_ip == ip){
            //send error message to client
            send(client_socketfd,"error",strlen("error"), 0);
            //close connection with client
            close(client_socketfd);
            return false;
        }
    }
    //check whether user is present in users vector
    for(auto user : users){
        if(user == current_user){
            //send success message to client
            send(client_socketfd, "AUTHENTICATED", sizeof("AUTHENTICATED"), 0);
            return true;
        }
    }
    //send failure message to client
    send(client_socketfd, "NOT_AUTHENTICATED", sizeof("NOT_AUTHENTICATED"), 0);
    return false;
}

void Server::createUser(int client_socketfd,User current_user){
    //check whether user is present in users vector
    for(auto user : users){
        if(user == current_user){
            //send failure message to client
            send(client_socketfd, "USER_ALREADY_EXISTS", sizeof("USER_ALREADY_EXISTS"), 0);
            return;
        }
    }
    //add user to users vector
    users.push_back(current_user);
    //store the data in ../data/users.txt file
    current_user.storeData();
    //create user's directory
    mkdir(current_user.getDir().c_str(), 0777);
    //send success message to client
    send(client_socketfd, "USER_CREATED", sizeof("USER_CREATED"), 0);
}

void Server::listDirContents(int client_socketfd,std::string directory){
    //list directory contents
    DIR *dir;
    struct dirent *ent;
    std::string buffer,filename;
    buffer.clear();
    if ((dir = opendir (directory.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == DT_DIR){
                filename.clear();
                //send the directory contents to client
                filename = "d******\t" + std::string(ent->d_name) + "\n";
                buffer += filename;
            }
            else if(ent->d_type == DT_REG){
                filename.clear();
                filename = "-******\t" + std::string(ent->d_name) + "\n";
                buffer += filename;
            }
        }
        //remove the last newline character
        buffer.pop_back();
        send(client_socketfd, buffer.c_str(), buffer.size(), 0);
        closedir(dir);
    }
    else
        send(client_socketfd, "NO_FILES_FOUND", sizeof("NO_FILES_FOUND"), 0);
}

void Server::changeDir(std::string new_directory,User* current_user,int client_socketfd){
    
    DIR *dir;
    struct dirent *ent;
    if(new_directory.empty()){
        //change directory to home directory
        current_user->changeDir("");
        //send switched to home directory message to client
        send(client_socketfd, "SWITCHED_TO_HOME_DIRECTORY", sizeof("SWITCHED_TO_HOME_DIRECTORY"), 0);
    }
    else if ((dir = opendir ((current_user->getDir()).c_str())) != NULL){
        while ((ent = readdir (dir)) != NULL){
            if(ent->d_type == DT_DIR && ent->d_name == new_directory && new_directory != ".." && new_directory != "."){
                    //change directory
                    current_user->changeDir(current_user->getDir()+ "/" + new_directory);
                    //send success message to client
                    send(client_socketfd, "DIRECTORY_CHANGED", sizeof("DIRECTORY_CHANGED"), 0);
                    closedir(dir);
                    return;
            }
            else if(new_directory == ".." || new_directory == "../"){
                //change directory and restrict user to go beyond the ../data/home/username directory
                if(current_user->getDir() == "../data/home/" + current_user->getName()){
                    //send restricted message to client
                    send(client_socketfd, "DIRECTORY_RESTRICTED", sizeof("DIRECTORY_RESTRICTED"), 0);
                    closedir(dir);
                    return;
                }
                //go to previous directory and remove the last directory from the current directory
                current_user->changeDir(current_user->getDir().substr(0,current_user->getDir().find_last_of("/")));
                //send success message to client
                send(client_socketfd, "DIRECTORY_CHANGED", sizeof("DIRECTORY_CHANGED"), 0);
                closedir(dir);
                return;
            }
        }
        closedir(dir);
    }
    //send failure message to client
    else
    send(client_socketfd, "DIRECTORY_NOT_FOUND", sizeof("DIRECTORY_NOT_FOUND"), 0);    
}

void Server::selectFile(std::string &filename,std::string dirname,int client_socketfd){
    //check whether file exists in current directory
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirname.c_str())) != NULL){
        while ((ent = readdir (dir)) != NULL){
            if(ent->d_type == DT_REG && std::string(ent->d_name) == filename){
                //set the filename to fully qualified path
                filename = dirname + "/" + filename;
                //send success message to client
                send(client_socketfd, "FILE_SELECTED", sizeof("FILE_SELECTED"), 0);
                std::cout << "File selected: " << filename << std::endl;
                closedir(dir);
                return;
            }
        }
        closedir(dir);
    }
    //clear filename
    filename.clear();
    //send failure message to client
    send(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"), 0);
}

void Server::editLine(int client_socketfd,std::string filename,int line_number){
    //open file in read mode at line_number line
    std::ifstream file(filename);
    if(!file.is_open()){
        //display error to stderr
        sendDataToClient(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"));
        return;
    }
    //store the file in a vector
    std::vector<std::string> lines;
    std::string line;
    while(std::getline(file,line)){
        lines.push_back(line);
    }
    //close file
    file.close();
    //check whether line_number is valid
    if(line_number > lines.size()){
        //send failure message to client
        sendDataToClient(client_socketfd, "INVALID_LINE_NUMBER", sizeof("INVALID_LINE_NUMBER"));
        return;
    }

    //send the selected line with line_number to client
    line.clear();
    line = std::to_string(line_number) + ":" + lines[line_number-1];
    sendDataToClient(client_socketfd, line.c_str(), line.size());

    //receive the edited line from client
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socketfd, buffer, sizeof(buffer), 0);
    //replace the line in the vector
    lines[line_number-1] = buffer;
    //open file in write mode
    std::ofstream file_write(filename);
    //write the vector to file
    for(auto line : lines){
        file_write << line << std::endl;
    }
    //close file
    file_write.close();    
}

void Server::viewFile(int client_socketfd,std::string filename,int start_line,int end_line){
    //open file in read mode
    std::ifstream file(filename);
    if(!file.is_open()){
        //send failure message to client
        sendDataToClient(client_socketfd, "FILE_NOT_FOUND", sizeof("FILE_NOT_FOUND"));
        return;
    }
    //read file contents
    std::string line;
    int i = 1;
    while(std::getline(file,line)){
        if(i >= start_line){
            //send line with line number to client
            std::string line_with_number = std::to_string(i) + " " + line + "\n";
            send(client_socketfd, line_with_number.c_str(), strlen(line_with_number.c_str()), 0);
            if(end_line != -1 && i == end_line){
                file.close();
                break;
            }
        }
        i++;
    }
    file.close();
    // // send error message to client
    // if(start_line > i){
    //     std::string err = "INVALID_LINE_NUMBER";
    //     send(client_socketfd, err.c_str(), strlen(err.c_str()), 0);
    // }
    // else{
    //     file.close();
    // }  

    //************* working ******************************************
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    // //read index.html and send it to the client
    // int mlen = 0;
    // int fp = open(filename.c_str(), O_RDONLY);
    // while(mlen = read(fp,buff,sizeof(buff))){
    //     write(client_socketfd, buff,mlen);
    //     memset(buff, 0, sizeof(buff));
    // }
	write(client_socketfd,buff,1024);
    // close(fp);
    //*****************************************************************
}
