//TCP server program
#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>
#include<sstream>
#include<server.h>

int main(){
    //create server object
    Server server;
    while(1){
        int clientfd = server.acceptConnections();
        //send confirmation message to client
        server.sendDataToClient(clientfd, "Connected to server",20); 

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
            std::string command,name,password,dir,filename = "";
            ss >> command >> name >> password;
            //display the username and password
            std::cout << data << std::endl;

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
                    else if(command =="create"){
                        std::string username,passwd;
                        ss >> username >> passwd;
                        std::cout << username << " " << passwd << std::endl;
                        server.createUser(clientfd,User(username,passwd));
                    }
                    else if(command == "pwd"){
                        server.sendDataToClient(clientfd,dir,dir.length());
                    }
                    else if(command == "cd"){
                        std::string new_dir;
                        ss >> new_dir;
                            server.changeDir(new_dir,&current_user,clientfd);
                    }
                    else if(command == "edit"){
                        int line_number;
                        ss >> line_number;
                        if(filename.empty())
                            server.sendDataToClient(clientfd,"0",1);
                        else
                        server.editLine(clientfd,filename,line_number);
                    }
                    else if(command == "print"){
                        if(filename.empty())
                            server.sendDataToClient(clientfd,"0",1);
                        else{
                            int start_line = 1,end_line = -1;
                            ss >> start_line >> end_line;
                            //print start_line and end_line
                            std::cout << "start_line: " << start_line << " end_line: " << end_line << std::endl;
                            server.viewFile(clientfd,filename,start_line,end_line);
                        }
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
            signal(SIGCHLD,SIG_IGN);
            close(clientfd);
        }
    }
    return 0;
}
