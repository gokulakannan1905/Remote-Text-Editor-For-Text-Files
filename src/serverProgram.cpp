/* TCP server driver program */

#include <iostream>
#include <string.h>
#include <sstream>
#include <limits>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <server.h>
#include <exception>

int main()
{
    try{
    // create server object
    Server server;
    server.LoadUsersData();
    while (1)
    {
        // accept connection from client and store the socket descriptor
        int clientfd = server.AcceptConnections();

        // send confirmation message to client
        server.SendDataToClient(clientfd, "Connected to server");

        // create child process
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        else if (pid == 0)
        {
            // receive data from client
            std::string data = server.ReceiveDataFromClient(clientfd);

            std::stringstream ss(data);
            std::string command, name, password, dir, filename;

            ss >> command >> name >> password;

            User current_user(name, password);

            /*
             * AUTHENTICATE current_user
             */
            if (server.AuthenticateUser(clientfd, current_user))
            {
                while (1)
                {
                    /*
                     * clear the buffer
                     */

                    data.clear();
                    ss.clear();
                    dir = current_user.GetDir();

                    /*
                     * Receive request from the client
                     */

                    data = server.ReceiveDataFromClient(clientfd);
                    ss.str(data);
                    ss >> command;

                    /*
                     * Command List
                     * Ls
                     * cd
                     * edit
                     * view
                     * select
                     * bye
                     */

                    if (command == "ls")
                    {
                        server.ListDirContents(clientfd, dir);
                    }
                    else if (command == "pwd")
                    {
                        server.SendDataToClient(clientfd, dir);
                    }
                    else if (command == "cd")
                    {
                        std::string new_dir;
                        ss >> new_dir;
                        server.ChangeDir(new_dir, current_user, clientfd);
                    }
                    else if (command == "edit")
                    {
                        int line_number;
                        ss >> line_number;
                        if (filename.empty())
                            server.SendDataToClient(clientfd, "0");
                        else
                            server.EditLine(clientfd, filename, line_number);
                    }
                    else if (command == "print")
                    {
                        if (filename.empty())
                            server.SendDataToClient(clientfd, "0");
                        else
                        {
                            int start_line = 1, end_line = -1;
                            ss >> start_line >> end_line;
                            server.ViewFile(clientfd, filename, start_line, end_line);
                        }
                    }
                    else if (command == "select")
                    {
                        ss >> filename;
                        if (!filename.empty())
                            server.SelectFile(filename, dir, clientfd);
                    }
                    else if (command == "bye")
                    {
                        close(clientfd);
                        break;
                    }
                    else
                    {
                        server.SendDataToClient(clientfd, "Invalid command");
                    }
                }
            }
        }
        else
        {
            /*
             * inside parent process
             * SIGCHLD is sent to the parent process when a child process exits.
             * closing the socket.
             */
            signal(SIGCHLD, SIG_IGN);
            close(clientfd);
        }
    }
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
