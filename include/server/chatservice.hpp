#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include<unordered_map>
#include<functional>
#include<muduo/net/TcpServer.h>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessage.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include<mutex>
using namespace std;
using json = nlohmann::json;

//表示处理消息的回调事件方法类型
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time)>;

class Chatservice{

public:

    //获取单例对象的借口函数
    static Chatservice* instance();

    //登陆业务需求
    void login(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);


    //注册业务需求
    void region(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);


    //点对点聊天服务
    void one_chat(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //处理客户端异常退出
    void clientquitexpection(const muduo::net::TcpConnectionPtr &conn);

    //添加好友业务
    void add_friend(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //创建群组业务
    void create_group(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //加入群组业务
    void add_group(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //群组聊天业务
    void group_chat(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //登出业务
    void login_out(const muduo::net::TcpConnectionPtr &conn,json &js,muduo::Timestamp time);

    //从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

    //服务器异常推出
    void reset();

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:

    Chatservice();

    unordered_map<int,MsgHandler> _msgHandlerMap;

    unordered_map<int,muduo::net::TcpConnectionPtr> _connmap;

    mutex _connmutex;

    UserModel _usermodel;

    offlinemsg _offlinemsgmodel;

    friendmodel _friendmodel;

    groupmodel _groupmodel;

    Redis _redis;
};
#endif