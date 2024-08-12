#ifndef USER_H
#define USER_H

#include<string>
using namespace std;

//User表的ORM类
class User{
public:
    User(int _id=-1,string _name="",string _pwd="",string _state="offline")
    :id(_id),name(_name),password(_pwd),state(_state){}

    void setid(int id){this->id = id;}
    void setname(string name){this->name = name;}
    void setpassword(string password){this->password = password;}
    void setstate(string state){this->state = state;}

    int getid(){return this->id;}
    string getname(){return this->name;}
    string getpassword(){return this->password;}
    string getstate(){return this->state;}

private:
    int id;
    string name;
    string password;
    string state;
};

#endif