/*
* This class is used to store user data.
*/
#ifndef USER_H
#define USER_H

#include <string>

class User
{
private:
    std::string name;
    std::string password;
    std::string dir;

public:
    User(std::string name, std::string password);
    void ChangeDir(std::string new_dir);
    std::string GetDir();
    bool operator==(const User &user);
    void StoreData();
    std::string GetName();
};

#endif
