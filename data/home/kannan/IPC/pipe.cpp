//sample pipe program
#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include<unistd.h>

int main(){
    //pipe program 
    //create pipe
    int pipefd[2];
    if(pipe(pipefd) == -1){
        std::cerr << "Error in creating pipe" << std::endl;
        exit(1);
    }
    //fork
    pid_t pid = fork();
    if(pid == -1){
        std::cerr << "Error in forking" << std::endl;
        exit(1);
    }
    if(pid == 0){
        //child process
        //close read end of pipe
        close(pipefd[0]);
        //write to pipe
        std::string data = "hello";
        write(pipefd[1], data.c_str(), data.size());
    }else{
        //parent process
        //close write end of pipe
        close(pipefd[1]);
        //read from pipe
        char buffer[100];
        read(pipefd[0], buffer, 100);
        std::cout << buffer << std::endl;
    }
    return 0;
}