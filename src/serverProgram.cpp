#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>
#include<sstream>
#include "../include/server.h"

int main(){
    //create server object
    Server server;
    while(1){
        int clientfd = server.acceptConnections();

        //create child process
        pid_t pid = fork();
        if(pid < 0){
            std::cerr << "Error in creating child process" << std::endl;
            exit(1);
        }
        else if(pid ==0){
            //receive data from client
            std::string data = server.receiveDataFromClient(clientfd);
            std::stringstream ss(data);
            std::string command,name,password,dir,filename;
            ss >> command >> name >> password;
            User current_user(name,password);
            if(server.authenticateUser(clientfd,current_user)){
                while(1){
                    data.clear();
                    ss.clear();
                    dir = current_user.getDir();
                    
                    //receive data from client
                    data = server.receiveDataFromClient(clientfd);
                    ss.str(data);
                    ss >> command;
                    std::cout << "data received: #" << command <<"#"<< std::endl;
                    if(command == "ls"){
                        server.listDirContents(clientfd,dir);
                    }
                    else if(command == "cd"){
                        std::string new_dir;
                        ss >> new_dir;
                        if(!new_dir.empty())
                            server.changeDir(new_dir,&current_user,clientfd);
                    }
                    else if(command == "edit"){
                        int line_number;
                        ss >> line_number;
                        server.editLine(clientfd,dir,filename,line_number);
                    }
                    else if(command == "print"){
                        int start_line = 1,end_line = -1;
                        ss >> start_line >> end_line;
                        server.viewFile(clientfd,dir,start_line,end_line);
                    }
                    else if(command == "select"){
                        ss >> filename;
                        if(!filename.empty())
                            server.selectFile(filename,dir,clientfd);                        
                    }
                    else if(command == "bye"){
                        close(clientfd);
                        break;
                    }
                    else{
                        server.sendDataToClient(clientfd,"Invalid command",strlen("Invalid command"));
                    }
                }
            }
        }
        else{
            //close connection
            close(clientfd);
        }
    }
    return 0;
}