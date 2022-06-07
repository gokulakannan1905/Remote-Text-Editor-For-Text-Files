/*
 * This is the source code for the client program.
 */

#include <iostream>
#include <string.h>
#include <sstream>
#include <limits>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <client.h>

/* This Global variable is used for signal handler function */
int sockid;

/* Function to handle ctrl+c */
void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        // send bye to server
        write(sockid, "bye", 3);
        exit(0);
    }
}

int main()
{
    while (true)
    {
        Client client;
        sockid = client.GetSocketfd();
        client.CreateSocket();

        /* register the signal handler for ctrl+c */
        signal(SIGINT, signalHandler);

        /* connect to server */
        client.ConnectToServer();

        /* receive connection confirmation from server */
        client.ReceiveDataFromServer();

        /* get username and password from user */
        std::string username, password;
        std::cout << "Enter username: ";
        std::getline(std::cin, username);

        termios oldt;
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        std::cout << std::endl;
        if (!(username.empty() && password.empty()))
        {
            /* hash the password using std::hash algorithm and convert it to string */
            std::hash<std::string> hash_fn;
            std::stringstream ss;
            ss << hash_fn(password);
            ss >> password;
        }
        else
        {
            username = "anonymous";
            password = "password";
        }
        /* authenticate user */
        if (client.AuthenticateUser(username, password))
        {
            if (username != "anonymous")
                std::cout << "AUTHENTICATED" << std::endl;
            while (true)
            {
                /* get user input */
                std::string input;
                std::cout << "Enter command-$: ";
                std::getline(std::cin, input);

                std::stringstream ss(input);
                int arguments = -1;
                std::string command, oneword;

                while (ss >> oneword)
                {
                    command += oneword + " ";
                    arguments++;
                }
                /* remove trailing space */
                command.replace(command.find_last_not_of(' ') + 1, command.length(), "");

                /* separate command and arguments */
                std::string subcommand = command.substr(0, command.find(" "));

                if ((subcommand == "ls" || subcommand == "pwd") && arguments == 0)
                    client.SendDataToServer(command);
                else if (subcommand == "cd" && arguments <= 1)
                    client.SendDataToServer(command);
                else if (subcommand == "print" && arguments <= 2)
                {
                    if (arguments == 0)
                        client.SendDataToServer(command);
                    else if (arguments == 1)
                    {
                        /* check 2nd argument is a number */
                        std::stringstream str_stream(command.substr(command.find(" ") + 1));
                        int number;
                        str_stream >> number;
                        if (str_stream.fail())
                        {
                            std::cerr << "Value must be a number" << std::endl;
                            continue;
                        }
                        if (number <= 0)
                        {
                            std::cerr << "Value should be greater than 0" << std::endl;
                            continue;
                        }
                        /* send command to server */
                        client.SendDataToServer(command);
                    }
                    else
                    {
                        /* check both 2nd and 3rd arguments are numbers */
                        int number1, number2;
                        std::stringstream ss1(command.substr(command.find(" ") + 1));
                        ss1 >> number1;
                        if (ss1.fail())
                        {
                            std::cerr << "Value 1 must be a number" << std::endl;
                            continue;
                        }
                        if (number1 <= 0)
                        {
                            std::cerr << "Value 1 should be greater than 0" << std::endl;
                            continue;
                        }
                        ss1 >> number2;
                        if (ss1.fail())
                        {
                            std::cerr << "Value 2 must be a number" << std::endl;
                            continue;
                        }
                        if (number2 <= 0)
                        {
                            std::cerr << "Value 2 should be greater than 0" << std::endl;
                            continue;
                        }
                        /* send command to server */
                        client.SendDataToServer(command);
                    }
                }
                else if (subcommand == "edit" && arguments == 1)
                {
                    /* check whether the argument is a number */
                    std::stringstream ss3(command.substr(command.find(" ") + 1));
                    int number;
                    ss3 >> number;
                    if (ss3.fail())
                    {
                        std::cerr << "Value must be a number" << std::endl;
                        continue;
                    }
                    if (number <= 0)
                    {
                        std::cerr << "Value should be greater than 0" << std::endl;
                        continue;
                    }
                    client.SendDataToServer(command);
                }
                else if (subcommand == "select" && arguments == 1)
                    client.SendDataToServer(command);
                else if (subcommand == "bye" && arguments == 0)
                {
                    client.DisconnectClient();
                    break;
                }
                else if (subcommand == "clear" || subcommand == "c")
                {
                    system("clear");
                    continue;
                }
                else
                {
                    if (subcommand == "ls" && arguments > 0)
                        std::cerr << "ls : too many arguments" << std::endl;
                    else if (subcommand == "cd" && arguments > 1)
                        std::cerr << "cd : too many arguments" << std::endl;
                    else if (subcommand == "print" && (arguments > 2))
                        std::cerr << "print : too many arguments" << std::endl;
                    else if (subcommand == "edit" && (arguments > 1 || arguments < 1))
                    {
                        if (arguments > 1)
                            std::cerr << "edit : too many arguments" << std::endl;
                        else
                            std::cerr << "<LINENUM> is missing in command : edit <LINENUM>" << std::endl;
                    }
                    else if (subcommand == "select" && (arguments > 1 || arguments < 1))
                    {
                        if (arguments > 1)
                            std::cerr << "select : too many arguments" << std::endl;
                        else
                            std::cerr << "<FILENAME> is missing in command : select <FILENAME>" << std::endl;
                    }
                    else if (subcommand == "bye" && arguments >= 1)
                        std::cerr << "bye : too many arguments" << std::endl;
                    else
                        std::cerr << "Invalid command" << std::endl;
                    continue;
                }

                /* receive data from server */
                if (subcommand == "edit")
                    client.EditLine();
                else if (subcommand == "print")
                    client.ReceiveFile();
                else
                    client.ReceiveDataFromServer();
            }
        }
        else
        {
            std::cerr << "AUTHENTICATION_FAILED try again...\n"
                      << std::endl;
            client.DisconnectClient();
        }
    }
    return 0;
}
