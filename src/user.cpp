/* This is the user class's implementation of member functions. */

#include <user.h>
#include <fstream>
#include <iostream>
#define DIR "../data/home/"
/*
 * Constructor for the user class.
 */
User::User(const std::string &name, const std::string &password)
{
    this->name = name;
    this->password = password;
    this->dir = DIR + name;
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
int User::ChangeDir(const std::string &dir)
{
    if (dir.empty())
    this->dir = DIR + name;
    else
    this->dir = dir;
    return 0;
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
