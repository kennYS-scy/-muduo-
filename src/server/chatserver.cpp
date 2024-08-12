#include "chatserver.hpp"
#include "json.hpp"
#include <functional>
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
//构造函数初始化聊天服务器对象
ChatServer::ChatServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenAddr,
                       const muduo::string &nameArg)
    : _server(loop, listenAddr, nameArg),
      loop(loop)
{
    //注册连接事件的回调函数
    _server.setConnectionCallback(std::bind(&ChatServer::onconnect, this, _1));

    //注册读写事件的回调函数
    _server.setMessageCallback(std::bind(&ChatServer::massagecb, this, _1, _2, _3));

    //设置线程数量 一个主reactor接收连接请求，其他线程接收读写请求
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onconnect(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        Chatservice::instance()->clientquitexpection(conn);
        conn->shutdown();
    }
}

void ChatServer::massagecb(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buffer,
                           muduo::Timestamp time)
{   
    string buf = buffer->retrieveAllAsString();
    
    //数据反序列化
    json js = json::parse(buf);

    //获取对应的事件处理器
    auto handler = Chatservice::instance()->getHandler(js["msgid"].get<int>());


    //调用相应的事件处理器
    handler(conn,js,time);
}