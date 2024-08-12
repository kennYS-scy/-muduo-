#include "chatservice.hpp"
#include "public.hpp"
#include<muduo/base/Logging.h>
#include<vector>
#include<iostream>
using namespace placeholders;

Chatservice* Chatservice::instance()
{
    static Chatservice service;

    return &service;
}

Chatservice::Chatservice()
{
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&Chatservice::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&Chatservice::region,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&Chatservice::one_chat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND,std::bind(&Chatservice::add_friend,this,_1,_2,_3)});
    _msgHandlerMap.insert({CREATE_GROUP,std::bind(&Chatservice::create_group,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_ADD_MEMBER,std::bind(&Chatservice::add_group,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT,std::bind(&Chatservice::group_chat,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGIN_OUT,std::bind(&Chatservice::login_out,this,_1,_2,_3)});
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&Chatservice::handleRedisSubscribeMessage, this, _1, _2));
    }

}

MsgHandler Chatservice::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);

    if(it == _msgHandlerMap.end())
    {
        return [=](const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time){
            LOG_ERROR<<"msgid"<<msgid<<"can not find handler!";
        };
    }
    else{
        return _msgHandlerMap[msgid];
    } 
}

void Chatservice::login(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _usermodel.query(id);

    if(user.getid()!=-1 &&user.getpassword() == pwd)
    {

        if(user.getstate() == "online")
        {
            json response;

            response["msgid"] = LOGIN_MSG_ACK;

            response["errno"] = 2;

            response["errmsg"] = "请勿重复登陆!";

            conn->send(response.dump());
        }
        else{

            //登陆成功，修改状态
            user.setstate("online");
            _usermodel.updateState(user);

            {
                lock_guard<mutex> lock(_connmutex);
                _connmap.insert({id,conn});
            }

            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            json response;

            response["msgid"] = LOGIN_MSG_ACK;

            response["errno"] = 0;

            response["id"] =id;

            response["name"] =user.getname();

            
            vector<string> offlinemsg = _offlinemsgmodel.query(id);
            if(offlinemsg.size()>0)
            {
                response["offlinemsg"] = offlinemsg;
                _offlinemsgmodel.remove(id);
            }

            vector<User> myfriends;
            myfriends = _friendmodel.query(id);
            if(myfriends.size()>0)
            {
                vector<string> vec;
                for(int i = 0 ; i< myfriends.size();i++)
                {
                    json js;
                    js["id"] = myfriends[i].getid();
                    js["name"] = myfriends[i].getname();
                    js["state"] = myfriends[i].getstate();
                    vec.push_back(js.dump());
                }
                response["friends"] = vec;
            }

            vector<Group> mygroup;
            mygroup = _groupmodel.QueryGroup(id);
            if(mygroup.size()>0)
            {
                vector<string> vec;
                for(Group &group:mygroup)
                {
                    json groupjs;
                    groupjs["id"] = group.getid();
                    groupjs["name"] = group.getname();
                    groupjs["desc"] = group.getdesc();
                    
                    vector<string> user;
                    for(groupuser &gu:group.getusers())
                    {
                        json js;
                        js["id"] = gu.getid();
                        js["name"] = gu.getname();
                        js["state"] = gu.getstate();
                        js["role"] = gu.getrole();
                        string str = js.dump();
                        user.push_back(str);
                    }
                    groupjs["groupuser"] = user;
                    vec.push_back(groupjs.dump());
                }
                response["group"] = vec;
            }

            conn->send(response.dump());
        }
    }
    else{
        json response;

        response["msgid"] = LOGIN_MSG_ACK;

        response["errno"] = 1;

        response["errmsg"] = "用户名或密码错误!";

        conn->send(response.dump());
    }
}

void Chatservice::region(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setname(name);
    user.setpassword(pwd);

    if(_usermodel.insert(user))
    {
        json response;

        response["msgid"] = REG_MSG_ACK;

        response["errno"] = 0;

        response["id"] = user.getid();

        conn->send(response.dump());
    }
    else{
        json response;

        response["msgid"] = REG_MSG_ACK;

        response["errno"] = 1;

        conn->send(response.dump());
    }
}

void Chatservice::clientquitexpection(const muduo::net::TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connmutex);

        for(auto it = _connmap.begin();it!=_connmap.end();it++)
        {
            if(it->second==conn)
            {
                 user.setid(it->first);
                _connmap.erase(it);
                break;
            }
        }
    }

    user.setstate("offline");
    _usermodel.updateState(user);
    LOG_INFO<<"用户异常退出";
}

void Chatservice::one_chat(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int toid = js["to"];

    {
        lock_guard<mutex> lock(_connmutex);

        auto it = _connmap.find(toid);

        //在线聊天
        if(it!=_connmap.end())
        {
            //toid在线 服务器帮忙推送消息
            it->second->send(js.dump());
            return;
        }
    }


    User user = _usermodel.query(toid);
    if(user.getstate()=="online")
    {
        _redis.publish(toid, js.dump());
        return;
    }


    _offlinemsgmodel.insert(toid,js.dump());

}

void Chatservice::reset()
{
    _usermodel.reset();
}

void Chatservice::add_friend(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int userid = js["id"];
    int friendid = js["friendid"];

    _friendmodel.insert(userid,friendid);
}

void Chatservice::create_group(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int userid = js["id"];
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];

    Group group(-1,groupname,groupdesc);
    _groupmodel.CreateGroup(group);
    _groupmodel.AddNumber(group.getid(),userid,"creator");
}

void Chatservice::add_group(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int groupid = js["groupid"];
    int userid = js["id"];
    string role = "normal";

    _groupmodel.AddNumber(groupid,userid,role);
}

void Chatservice::group_chat(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int groupid = js["groupid"];
    int id = js["id"];

    vector<int> groupmember;

    groupmember = _groupmodel.MemberQuery(id,groupid);

    for(auto &i : groupmember)
    {
        {
            lock_guard<mutex> lock(_connmutex);
            auto it = _connmap.find(i);
            if(it!=_connmap.end())
            {
                it->second->send(js.dump());
                return;
            }
        }

        User user = _usermodel.query(i);
        if(user.getstate()=="online")
        {
            _redis.publish(i, js.dump());
            return;
        }


        _offlinemsgmodel.insert(i,js.dump());
    }
    
}

void Chatservice::login_out(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)
{
    int id = js["id"];

    User user(id,"","","offline");
    _usermodel.updateState(user);

    {
        lock_guard<mutex> lock(_connmutex);

        for(auto it = _connmap.begin();it!=_connmap.end();it++)
        {
            if(it->second==conn)
            {
                _connmap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(id);
}

void Chatservice::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connmutex);
    auto it = _connmap.find(userid);
    if (it != _connmap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlinemsgmodel.insert(userid, msg);
}