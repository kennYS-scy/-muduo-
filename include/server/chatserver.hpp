#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>




//聊天服务器主类
class ChatServer{

private:

    //初始化聊天服务器对象
    muduo::net::TcpServer _server;

    //初始化事件指针
    muduo::net::EventLoop *loop;

    //上报连接的回调函数
    void onconnect(const muduo::net::TcpConnectionPtr& );
    
    //上报读写事件的回调函数
    void massagecb(const muduo::net::TcpConnectionPtr&,
                           muduo::net::Buffer*,
                            muduo::Timestamp );

public:
    
    //构造函数初始化聊天服务器对象
    ChatServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const muduo::string& nameArg);

    //启动监听
    void start();

};


#endif