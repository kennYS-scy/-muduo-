#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"
class groupuser: public User
{
public:
    void setrole(string _role){this->role = _role;}

    string getrole(){return role;}
private:
    string role;
};

#endif