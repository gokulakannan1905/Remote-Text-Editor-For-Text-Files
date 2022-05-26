#include "../include/user.h"
#include<fstream>
#include<iostream>
User::User(std::string name, std::string password){
    this->name = name;
    this->password = password;
    this->dir = "../data/home/" + name;
}

void User::changeDir(std::string dir){
    this->dir = dir;
}

std::string User::getDir(){
    return this->dir;
}

bool User::operator==(User user){
    return (this->name == user.name && this->password == user.password);
}

void User::storeData(){
    std::ofstream file;
    file.open("../data/users.txt", std::ios::app);
    //handle the error if file is not opened
    if(!file.is_open()){
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    //store the user name : password : dir in the file
    file << this->name << ":" << this->password << ":" << this->dir << std::endl;
    file.close();
}