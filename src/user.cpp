/* This is the user class's implementation of member functions. */

#include "../include/user.h"
#include <fstream>
#include <iostream>

/*
 * Constructor for the user class.
 */
User::User(std::string name, std::string password)
{
    this->name = name;
    this->password = password;
    this->dir = "../data/home/" + name;
}


/*
 * Returns the name of the user.
 */
std::string User::GetName()
{
    return this->name;
}

/*
 * changes the directory of the user.
 */
void User::ChangeDir(std::string dir)
{
    if (dir.empty())
    {
        this->dir = "../data/home/" + name;
    }
    else
    {
        this->dir = dir;
    }
}

/*
 * Returns the directory of the user.
 */
std::string User::GetDir()
{
    return this->dir;
}

/*
 * overloading == operator to compare two users.
 */
bool User::operator==(const User &user)
{
    return (this->name == user.name && this->password == user.password);
}

/*
 * to store the data of the user in a file.
 */
void User::StoreData()
{
    std::ofstream file;
    file.open("../data/users.txt", std::ios::app);
    // handle the error if file is not opened
    if (!file.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    // store the user name : password : dir in the file
    file << this->name << " " << this->password << " " << this->dir << std::endl;
    file.close();
}