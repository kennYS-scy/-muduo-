#include "usermodel.hpp"
#include "db.hpp"
#include<iostream>

bool UserModel::insert(User &user)
{
    char buf[1024] = {0};
    sprintf(buf,"insert into User(name,password,state) values('%s','%s','%s')",
                user.getname().c_str(),user.getpassword().c_str(),user.getstate().c_str());

    MySQL mysql;

    if(mysql.connect())
    {
        if(mysql.update(buf))
        {
            //获取插入成功的用户数据生成的主键id,因为传入的user参数是没有ID这个字段的
            user.setid(mysql_insert_id(mysql.getconnect()));
            return true;
        }
    }

    return false;
}

User UserModel::query(int id)
{
    char buf[1024] = {0};
    sprintf(buf,"select * from User where id=%d",id);

    MySQL mysql; 

    if(mysql.connect())
    {
        MYSQL_RES* res; 
        res = mysql.query(buf);

        if(res!=nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setid(atoi(row[0]));
                user.setname(row[1]);
                user.setpassword(row[2]);
                user.setstate(row[3]);
                mysql_free_result(res);
                return user;
            }

        }
        mysql_free_result(res);
    }

    return User();
}

bool UserModel::updateState(User user)
{
    char buf[1024] = {0};
    sprintf(buf,"update User set state='%s' where id=%d",
                user.getstate().c_str(),user.getid());

    MySQL mysql;

    if(mysql.connect())
    {
        if(mysql.update(buf))
        {

            return true;
        }
    }
    return false;
}

void UserModel::reset()
{
    char buf[1024] = "update User set state='offline'";

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(buf);
    }
}