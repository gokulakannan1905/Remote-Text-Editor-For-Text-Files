#include<iostream>
#include<string.h>
#include<sstream>
#include<limits>
#include <termios.h>
#include <unistd.h>
#include "../include/client.h"

int main(){ 
    //create client object

    while(true){ 
    Client client;
    //connect to server
    client.connectToServer();
    //get username and password from user
    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::cout << "Enter password: ";
    std::cin >> password;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    //hash the password using std::hash algorithm and convert it to string 
    std::hash<std::string> hash_fn;
    std::stringstream ss;
    ss << hash_fn(password);
    ss >> password;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //authenticate user
    if(client.authenticateUser(username,password)){
        std::cout << "Authenticated" << std::endl;
        while(true){
            //get user input
            std::string input;
            std::cout << "Enter command: ";
            std::getline(std::cin, input);

            std::stringstream ss(input);
            int arguments = -1;
            std::string command,oneword;
            //arguments the number of words in the input
            while(ss >> oneword){
                command+=oneword+" ";
                arguments++;
            }
            command.replace(command.find_last_not_of(' ') + 1, command.length(), "");
            std::string subcommand = command.substr(0,command.find(" "));
            if(subcommand == "ls" && arguments == 0){
                client.sendDataToServer(command, strlen(command.c_str()));
            }
            else if(subcommand == "cd" && arguments <= 1){              
                client.sendDataToServer(command, strlen(command.c_str()));
            }
            else if(subcommand == "print" && arguments <= 2){
                client.sendDataToServer(command, strlen(command.c_str()));
            }
            else if(subcommand == "edit" && arguments == 1){                
                client.sendDataToServer(command, strlen(command.c_str()));
            }
            else if(subcommand == "select" && arguments == 1){              
                client.sendDataToServer(command, strlen(command.c_str()));
            }
            else if(subcommand == "bye" && arguments == 0){
                client.disconnectClient();
                exit(0);
            }
            else{
                if(subcommand == "ls" && arguments > 0){
                    std::cout << "ls : too many arguments" << std::endl;
                }
                else if(subcommand == "cd" && arguments > 1){
                    std::cout << "cd : too many arguments" << std::endl;
                }
                else if(subcommand == "print" && (arguments > 2)){
                        std::cout << "print : too many arguments" << std::endl;
                }
                else if(subcommand == "edit" && (arguments > 1 || arguments < 1)){
                    if(arguments > 1){
                        std::cout << "edit : too many arguments" << std::endl;
                    }
                    else{
                        std::cout << "<LINENUM> is missing in command : edit <LINENUM>" << std::endl;
                    }
                }
                else if(subcommand == "select" && (arguments > 1 || arguments < 1)){
                    if(arguments > 1){
                        std::cout << "select : too many arguments" << std::endl;
                    }
                    else{
                        std::cout << "<FILENAME> is missing in command : select <FILENAME>" << std::endl;
                    }
                }
                else if(subcommand == "bye" && arguments > 1){
                    std::cout << "bye : too many arguments" << std::endl;
                }
                else{
                    std::cout << "Invalid command" << std::endl;
                }
                continue; //continue the loop
            }
       
            //receive data from server
            client.receiveDataFromServer();
        }

    }   
    std::cout << "Authentication failed" << std::endl;
    client.disconnectClient();
    }
    return 0;
}