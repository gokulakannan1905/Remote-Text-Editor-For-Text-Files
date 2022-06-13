/*
* This class is used to store user data.
*/
#ifndef USER_H
#define USER_H

#include <string>

class User
{
public:
    User(const std::string &name,const std::string &password);
    int ChangeDir(const std::string &new_dir);
    std::string GetDir();
    bool operator==(const User &user);
    std::string GetName();
private:
    std::string name;
    std::string password;
    std::string dir;
};

#endif
