#include<iostream>
#include<string.h>
//this comment is added via line editor
using namespace std;
int main(){
    string input;
    cout << "Enter command: ";
    getline(cin,input);
    cout << "input: " << input << endl;
    stringstream ss(input);
    string command,name,password,dir,filename = "";
    ss >> command >> name >> password;
    cout << "command: " << command << endl;
    cout << "name: " << name << endl;
    cout << "password: " << password << endl;
    return 0;
}
