//Program to implement FIFO
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<cstdlib>
#include<sys/stat.h>

int main(){
	char fifoName[] = "fifotest";
	int fifoFD;
	char buf[1024];
	mkfifo(fifoName,0666);
	while(true){
		fifoFD = open(fifoName,O_RDONLY);
		//clear the buffer
		memset(buf,0,sizeof(buf));
		int length = read(fifoFD,buf,1024);
		//close the fifo if the string is "exit"
		write(1,"MSG from frnd : ",sizeof("MSG from frnd : "));
		write(1,buf,length);
		if(strcmp(buf,"exit\n")==0){
			close(fifoFD);
			//unlink(fifoName);
			exit(0);
		}
		close(fifoFD);

		fifoFD = open(fifoName,O_WRONLY);
		write(1,"Enter MSG : ",sizeof("Enter MSG : "));
		//clear the buffer
		memset(buf,0,sizeof(buf));
		length = read(0,buf,1024);
		//close the fifo if the string is "exit"
		write(fifoFD,buf,length);
		if(strcmp(buf,"exit\n")==0){
			close(fifoFD);
unlink(fifoName);
			exit(0);
		}
		close(fifoFD);
	}
}
