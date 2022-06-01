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

/* Global variable */
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
            std::cout << "Authenticated" << std::endl;
            while (true)
            {
                /* get user input */
                std::string input;
                std::cout << "Enter command: ";
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
                {
                    client.SendDataToServer(command, strlen(command.c_str()));
                }
                else if (subcommand == "create" && arguments == 2)
                {
                    /* get username and password from user */
                    std::string user_name, passwd;
                    std::stringstream str_s(command);
                    str_s >> subcommand >> user_name >> passwd;
                    client.CreateUser(user_name, passwd);
                    continue;
                }
                else if (subcommand == "cd" && arguments <= 1)
                {
                    client.SendDataToServer(command, strlen(command.c_str()));
                }
                else if (subcommand == "print" && arguments <= 2)
                {
                    if (arguments == 0)
                        client.SendDataToServer(command, strlen(command.c_str()));
                    else if (arguments == 1)
                    {
                        /* check 2nd argument is a number */
                        std::stringstream str_stream(command.substr(command.find(" ") + 1));
                        int number;
                        str_stream >> number;
                        if (ss.fail())
                        {
                            std::cout << "Invalid argument" << std::endl;
                            continue;
                        }
                        /* send command to server */
                        client.SendDataToServer(command, strlen(command.c_str()));
                    }
                    else
                    {
                        /* check both 2nd and 3rd arguments are numbers */
                        int number1, number2;
                        std::stringstream ss1(command.substr(command.find(" ") + 1));
                        ss1 >> number1;
                        if (ss1.fail())
                        {
                            std::cout << "Invalid argument" << std::endl;
                            continue;
                        }
                        std::stringstream ss2(command.substr(command.find(" ") + 1 + 1));
                        ss2 >> number2;
                        if (ss2.fail())
                        {
                            std::cout << "Invalid argument" << std::endl;
                            continue;
                        }
                        /* send command to server */
                        client.SendDataToServer(command, strlen(command.c_str()));
                    }
                }
                else if (subcommand == "edit" && arguments == 1)
                {
                    /* check whether the argument is a number */
                    std::stringstream ss3(command.substr(command.find(" ") + 1));
                    int number;
                    ss3 >> number;
                    if (ss.fail())
                    {
                        std::cout << "Invalid argument" << std::endl;
                        continue;
                    }
                    client.SendDataToServer(command, strlen(command.c_str()));
                }
                else if (subcommand == "select" && arguments == 1)
                {
                    client.SendDataToServer(command, strlen(command.c_str()));
                }
                else if (subcommand == "bye" && arguments == 0)
                {
                    client.DisconnectClient();
                    exit(0);
                }
                else if (subcommand == "clear" || subcommand == "cls" || subcommand == "c")
                {
                    system("clear");
                    continue;
                }
                else
                {
                    if (subcommand == "ls" && arguments > 0)
                    {
                        std::cerr << "ls : too many arguments" << std::endl;
                    }
                    else if (subcommand == "cd" && arguments > 1)
                    {
                        std::cerr << "cd : too many arguments" << std::endl;
                    }
                    else if (subcommand == "print" && (arguments > 2))
                    {
                        std::cerr << "print : too many arguments" << std::endl;
                    }
                    else if (subcommand == "edit" && (arguments > 1 || arguments < 1))
                    {
                        if (arguments > 1)
                        {
                            std::cerr << "edit : too many arguments" << std::endl;
                        }
                        else
                        {
                            std::cerr << "<LINENUM> is missing in command : edit <LINENUM>" << std::endl;
                        }
                    }
                    else if (subcommand == "select" && (arguments > 1 || arguments < 1))
                    {
                        if (arguments > 1)
                        {
                            std::cerr << "select : too many arguments" << std::endl;
                        }
                        else
                        {
                            std::cerr << "<FILENAME> is missing in command : select <FILENAME>" << std::endl;
                        }
                    }
                    else if (subcommand == "bye" && arguments >= 1)
                    {
                        std::cerr << "bye : too many arguments" << std::endl;
                    }
                    else
                    {
                        std::cerr << "Invalid command" << std::endl;
                    }
                    /* continue to next iteration */
                    continue;
                }

                /* receive data from server */
                if (subcommand == "edit")
                {
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    int bytes_read = read(sockid, buffer, sizeof(buffer));
                    if (strcmp("0", buffer) == 0)
                    {
                        std::cerr << "FILE_NOT_SELECTED: use select <FILENAME> command" << std::endl;
                        continue;
                    }
                    if (strcmp("INVALID_LINE_NUMBER", buffer) == 0)
                    {
                        std::cerr << "INVALID_LINE_NUMBER" << std::endl;
                        continue;
                    }
                    write(1, buffer, bytes_read);

                    /* ask user to edit the line received from server and send it to server */
                    std::string edited_line;
                    std::cout << "\nEnter changes to the line: ";
                    /* allow user to type empty line also */
                    std::getline(std::cin, edited_line);
                    if (edited_line.length() == 0)
                        edited_line = buffer;
                    client.SendDataToServer(edited_line, strlen(edited_line.c_str()));
                }
                else if (subcommand == "print")
                {
                    client.ReceiveFile();
                }
                else
                    client.ReceiveDataFromServer();
            }
        }

        /*
         * If the authentication fails, the client will be disconnected from the server
         */
        std::cerr << "Authentication failed" << std::endl;
        client.DisconnectClient();
    }
    return 0;
}
