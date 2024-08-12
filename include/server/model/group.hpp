#ifndef GROUP_H
#define GROUP_H
#include<string>
#include<vector>
#include "groupuser.hpp"
using namespace std;

class Group{

public:
    Group(int _id = -1,string _name="",string _desc=""):id(_id),name(_name),desc(_desc){}
    void setid(int id){this->id=id;}
    void setname(string name){this->name=name;}
    void setdesc(string desc){this->desc=desc;}

    int getid(){return id;}
    string getname(){return this->name;}
    string getdesc(){return this->desc;}
    vector<groupuser>& getusers(){return this->users;}
private:
    int id;
    string name;
    string desc;
    vector<groupuser> users;
};

#endif