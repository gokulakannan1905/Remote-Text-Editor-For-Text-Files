#include<iostream>  
#include<string.h>
#include<sstream>
#include<limits>
#include<signal.h>
#include<sys/types.h>
int main(){
    //program to demonstarte fifo
    //create fifo
    mkfifo("fifo",0666);
    //open fifo
    int fd = open("fifo",O_RDWR);
    //read from fifo
    char buff[100];
    read(fd,buff,100);
    std::cout << buff << std::endl;
    //write to fifo
    write(fd,"hello",5);
    close(fd);
    return 0;
    
}