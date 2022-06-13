/*
 * This is the source code for the client program.
 */

#include <iostream>
#include <exception>
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
        send(sockid, "bye", 3,0);
        exit(0);
    }
}

int main()
{
    while (true)
    {
        try
        {
            Client client;
            client.CreateSocket();
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
                if (username != "anonymous")
                    std::cout << "AUTHENTICATED" << std::endl;
                while (true)
                {
                    /* get user input */
                    std::string input;
                    std::cout << "Enter command-$: ";
                    std::getline(std::cin, input);
                    if (feof(stdin))
                    {
                        client.DisconnectClient();
                        exit(0);
                    }

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
                        client.SendDataToServer(command);
                    }
                    else if (subcommand == "help" && arguments == 0)
                    {
                        std::cout << "List of commands available: ls pwd cd select print edit clear bye help" << std::endl;
                        continue;
                    }
                    else if (subcommand == "help" && arguments == 1)
                    {
                        std::string help_command = command.substr(command.find(" ") + 1);
                        if (help_command == "ls")
                        {
                            std::cout << "ls - list files in current directory\nNo arguments are needed" << std::endl;
                        }
                        else if (help_command == "pwd")
                        {
                            std::cout << "pwd - print current directory\nNo arguments are needed" << std::endl;
                        }
                        else if (help_command == "cd")
                        {
                            std::cout << "cd <directory> - change current directory\nIt takes directory name as argument" << std::endl;
                        }
                        else if (help_command == "select")
                        {
                            std::cout << "select <filename> - select file to edit\nIt takes filename name as argument" << std::endl;
                        }
                        else if (help_command == "print")
                        {
                            std::cout << "print <start> <end> - print the file\nIt takes 2 numbers (starting and ending number) as optional arguments" << std::endl;
                        }
                        else if (help_command == "edit")
                        {
                            std::cout << "edit <linenum> - edit the file\nIt takes line number as arguments and edit the line on the selected file" << std::endl;
                        }
                        else if (help_command == "bye")
                        {
                            std::cout << "bye - logout the current user\nNo arguments are needed" << std::endl;
                        }
                        else if (help_command == "help")
                        {
                            std::cout << "help - print list of commands\nsubcommand are passed as arguments" << std::endl;
                        }
                        else if (help_command == "clear")
                        {
                            std::cout << "clear - clear the screen\nNo arguments are needed" << std::endl;
                        }
                        else
                        {
                            std::cout << "Invalid command" << std::endl;
                        }
                        continue;
                    }
                    else if (subcommand == "cd" && arguments <= 1)
                    {
                        client.SendDataToServer(command);
                    }
                    else if (subcommand == "print" && arguments <= 2)
                    {
                        if (arguments == 0)
                        {
                            client.SendDataToServer(command);
                        }
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
                    {
                        client.SendDataToServer(command);
                    }
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
                        continue;
                    }

                    /* receive data from server */
                    if (subcommand == "edit")
                    {
                        client.EditLine();
                    }
                    else if (subcommand == "print")
                    {
                        client.ReceiveFile();
                    }
                    else
                    {
                        client.ReceiveDataFromServer();
                    }
                }
            }
            else
            {
                std::cerr << "AUTHENTICATION_FAILED try again...\n"
                          << std::endl;
                client.DisconnectClient();
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    return 0;
}
