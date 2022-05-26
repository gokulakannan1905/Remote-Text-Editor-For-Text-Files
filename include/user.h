//avoid multiple inclusions
#ifndef USER_H
#define USER_H

#include<string>
class User{
    std::string name;
    std::string password;
    std::string dir;
    public:
    User(std::string,std::string);
    void changeDir(std::string);
    std::string getDir();
    bool operator==(User);
    void storeData();
};

#endif