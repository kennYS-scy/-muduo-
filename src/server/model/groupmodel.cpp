#include "groupmodel.hpp"
#include "db.hpp"
#include<string>
#include<iostream>
using namespace std;
void groupmodel::CreateGroup(Group &group)
{
    string group_name = group.getname();
    string group_desc = group.getdesc();

    char buf[1024] = {0};
    sprintf(buf,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",
            group_name.c_str(),group_desc.c_str());

    MySQL mysql;

    if(mysql.connect())
    {
        if(mysql.update(buf))
        {
            group.setid(mysql_insert_id(mysql.getconnect()));
        }
    }
}
void groupmodel::AddNumber(int GroupId,int userid,string role)
{
    char buf[1024] = {0};
    sprintf(buf,"insert into GroupUser(groupid,userid,grouprole) values('%d','%d','%s')",
                GroupId,userid,role.c_str());

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(buf);
    }
}
//查询用户所在群组信息
vector<Group> groupmodel::QueryGroup(int UserId)
{
    char buf[1024] = {0};
    sprintf(buf,"select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id=b.groupid where b.userid=%d"
                   ,UserId);

    MySQL mysql; 
    vector<Group> grouplist;

    if(mysql.connect())
    {
        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row= mysql_fetch_row(res))!=nullptr)
            {
                Group group;
                group.setid(atoi(row[0]));
                group.setname(row[1]);
                group.setdesc(row[2]);
                grouplist.push_back(group);
            }

        }
        mysql_free_result(res);
    }

    for(Group &temp:grouplist)
    {
        sprintf(buf,"select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.groupid=%d"
                   ,temp.getid());

        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row= mysql_fetch_row(res))!=nullptr)
            {
                groupuser user;
                user.setid(atoi(row[0]));
                user.setname(row[1]);
                user.setstate(row[2]);
                user.setrole(row[3]);
                temp.getusers().push_back(user);
            }

        }
        mysql_free_result(res);
    }
    return grouplist;
}


//查询所在群除本用户外的其他用户ID
vector<int> groupmodel::MemberQuery(int Userid,int GroupId)
{
    char buf[1024] = {0};
    sprintf(buf,"select userid from GroupUser where groupid=%d"
                   ,GroupId);

    MySQL mysql; 
    vector<int> idlist;

    if(mysql.connect())
    {
        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row= mysql_fetch_row(res))!=nullptr)
            {
                int id = atoi(row[0]);
                if(id == Userid)
                    continue;
                idlist.push_back(id);
            }

        }
        mysql_free_result(res);
    }
    return idlist;
}