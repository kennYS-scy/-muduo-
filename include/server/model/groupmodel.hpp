#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include<string>
#include "group.hpp"
#include<vector>
using namespace std;
class groupmodel{

public:

void CreateGroup(Group &group);

void AddNumber(int GroupId,int userid,string role);

vector<Group> QueryGroup(int UserId);

vector<int> MemberQuery(int Userid,int GroupId);
};

#endif