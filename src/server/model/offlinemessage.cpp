#include "offlinemessage.hpp"
#include "db.hpp"

//存储离线消息
void offlinemsg::insert(int userid,string msg)
{
    char buf[1024] = {0};
    sprintf(buf,"insert into OfflineMessage values('%d','%s')",
                userid,msg.c_str());

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(buf);
    }

}
//删除离线消息
void offlinemsg::remove(int userid)
{
    char buf[1024] = {0};
    sprintf(buf,"delete from OfflineMessage where userid=%d",
                userid);

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(buf);
    }
}

//查询离线消息
vector<string> offlinemsg::query(int userid)
{
    char buf[1024] = {0};
    sprintf(buf,"select message from OfflineMessage where userid=%d",userid);

    MySQL mysql; 
    vector<string> str;

    if(mysql.connect())
    {
        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row= mysql_fetch_row(res))!=nullptr)
            {
                str.push_back(row[0]);
            }

        }
        mysql_free_result(res);
    }

    return str;
}