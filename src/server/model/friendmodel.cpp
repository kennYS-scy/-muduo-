#include "friendmodel.hpp"
#include "db.hpp"

#include<vector>
using namespace std;

void friendmodel::insert(int userid,int friendid)
{
    char buf[1024] = {0};
    sprintf(buf,"insert into Friend values('%d','%d')",
                userid,friendid);

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(buf);
    }

}


vector<User> friendmodel::query(int userid)
{
    char buf[1024] = {0};
    sprintf(buf,"select a.id,a.name,a.state from User a inner join Friend b on a.id=b.friendid where b.userid=%d"
                   ,userid);

    MySQL mysql; 
    vector<User> friendlist;

    if(mysql.connect())
    {
        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row= mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setid(atoi(row[0]));
                user.setname(row[1]);
                user.setstate(row[2]);
                friendlist.push_back(user);
            }

        }
        mysql_free_result(res);
    }
    return friendlist;
}