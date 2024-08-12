#include"json.hpp"
#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include"arpa/inet.h"
#include "user.hpp"
#include "group.hpp"
#include "public.hpp"
#include<thread>
using namespace std;
using json=nlohmann::json;

//记录当前系统登录的用户信息
User g_currentUser;

//记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;

//记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

//显示当前登录成功用户的基本信息
void showCurrentUserData();

//接收线程
void readTaskHandler(int cfd);

//获取系统时间
string getCurrentTime();

//主聊天页面程序
void mainMenu(int cfd);

// "help" command handler
void Help(int fd = 0, string str = "");
// "chat" command handler
void Chat(int, string);
// "addfriend" command handler
void AddFriend(int, string);
// "creategroup" command handler
void CreateGroup(int, string);
// "addgroup" command handler
void AddGroup(int, string);
// "groupchat" command handler
void GroupChat(int, string);
// "loginout" command handler
void LoginOut(int, string);

//控制主菜单是否继续显示
bool g_is_menu_running = false;
// 系统支持的客户端命令列表
unordered_map<string, string> command_map = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> command_handler_map = {
    {"help", Help},
    {"chat", Chat},
    {"addfriend", AddFriend},
    {"creategroup", CreateGroup},
    {"addgroup", AddGroup},
    {"groupchat", GroupChat},
    {"loginout", LoginOut}};


int main(int argc,char **argv)
{
    if(argc<3)
    {
        cerr<<"command invalid! example: ./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int cfd = socket(AF_INET,SOCK_STREAM,0);

    if(cfd == -1)
    {
        cerr<<"socket create error"<<endl;
        exit(-1);
    }

    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    inet_pton(AF_INET,ip,&server_addr.sin_addr.s_addr);

    if(-1 == connect(cfd,(sockaddr *)&server_addr,sizeof(server_addr)))
    {
        cerr<<"socket connect error"<<endl;
        close(cfd);
        exit(-1);
    }

    //main线程用于接收用户输入，负责发送数据
    while(1)
    {
        //显示首页面菜单 登录 注册 退出
        cout <<"==================" <<endl;
        cout <<"1. login" <<endl;
        cout <<"2.register"<<endl;
        cout <<"3.quit"<<endl;
        cout <<"==================" <<endl;
        cout <<"choice:";
        int choice = 0;
        cin>>choice;
        cin.get(); //读掉缓冲区残留的回车
        g_currentUserFriendList.clear();
        g_currentUserGroupList.clear();
        switch(choice)
        {
            case 1: //login
            {
                int id = 0;
                cout<<"id:";
                cin>>id;
                cin.get();//读掉缓冲区的回车
                cout<<"password:";
                char buf[50] = {0};
                cin.getline(buf,50);

                json js;
                js["msgid"] = 1;
                js["id"] = id;
                js["password"] = buf;
                string request = js.dump();

                int len = send(cfd,request.c_str(),strlen(request.c_str())+1,0);

                if(len == -1)
                {
                    cerr<<"send login msg error"<<endl;
                }else{
                    char buffer[1024] = {0};

                    len = recv(cfd,buffer,1024,0);

                    json rejs = json::parse(buffer);

                    if(rejs["errno"]!=0)// 登录失败
                    {
                        cout<<rejs["errmsg"]<<endl;
                    }else{              //登录成功

                        cout<<"success login"<<endl;
                        g_currentUser.setid(rejs["id"]);
                        g_currentUser.setname(rejs["name"]);

                        if(rejs.contains("friends"))
                        {
                            vector<string> friends = rejs["friends"];

                            for(string &str : friends)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setid(js["id"]);
                                user.setname(js["name"]);
                                user.setstate(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                            
                        }
                        if(rejs.contains("group"))
                        {
                            vector<string> vec = rejs["group"];

                            for(string &str:vec)
                            {
                                json js = json::parse(str);
                                Group group;
                                group.setid(js["id"]);
                                group.setname(js["name"]);
                                group.setdesc(js["desc"]);
                                vector<string> group_user = js["groupuser"];
                                for(string &userstr:group_user)
                                {
                                    groupuser user;
                                    json groupjs = json::parse(userstr);
                                    user.setid(groupjs["id"]);
                                    user.setname(groupjs["name"]);
                                    user.setstate(groupjs["state"]);
                                    user.setrole(groupjs["role"]);
                                    group.getusers().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        
                        }
                        //显示登录用户的基本信息
                        showCurrentUserData();

                        //显示离线消息
                        if(rejs.contains("offlinemsg"))
                        {
                            vector<string> vec=rejs["offlinemsg"];
                            for(string &str:vec)
                            {
                                json js = json::parse(str);
                                int msgtype = js["msgid"];
                                if(msgtype == ONE_CHAT_MSG)
                                {
                                    cout<<js["time"]<<" id:"<<js["id"]<<"msg:"<<js["msg"]<<endl;
                                }else if(msgtype == GROUP_CHAT)
                                {
                                    cout<<"group" <<js["groupid"]<<"msg"<<js["time"]<<" id:"<<js["id"]<<"msg:"<<js["msg"]<<endl;
                                }
                            
                            }
                        }
                        
                        //登录成功，启动接收线程负责接收数据
                        static int tasknum = 0;
                        if(tasknum==0)
                        {
                            std::thread readTask(readTaskHandler,cfd);  //相当于phread_create
                            readTask.detach();
                            tasknum++;
                        }
                        g_is_menu_running = true;
                        //进入主菜单
                        mainMenu(cfd);
                    }
                }
                break;

            }
            case 2:     //注册业务
            {
                cout<<"name :";
                char name[50]={0};
                cin.getline(name,50);
                char pwd[50]={0};
                cin.getline(pwd,50);

                json js;
                js["msgid"] = 3;
                js["name"] = name;
                js["password"] = pwd;

                int len = send(cfd,js.dump().c_str(),strlen(js.dump().c_str())+1,0);

                if(len == -1)
                {
                    cerr<<"region error"<<endl;
                }else{
                    char buffer[1024] = {0};
                    int len = recv(cfd,buffer,sizeof(buffer),0);

                    json rejs;
                    rejs = json::parse(buffer);

                    if(rejs["errno"] == 1)
                    {
                        cerr<<"region fail,please reset id and pwd"<<endl;
                    }else{
                        cout<<"congratulate region success"<<endl;
                        cout<<"your id is:"<<rejs["id"]<<endl;
                    }

                }
                break;
            }
            case 3:
            {
                close(cfd);
                exit(0);
                break;
            }
            default:
            {
                cerr<<"invaild input!!!"<<endl;
                break;
            }
        }

    }

    return 0;
}


void showCurrentUserData()
{
    cout<<"========================login user=========================="<<endl;
    cout<<"current login user=-> id:"<<g_currentUser.getid()<<"name:"<<g_currentUser.getname()<<endl;
    cout<<"------------------------friend list-------------------------"<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(User &user:g_currentUserFriendList)
        {
            cout<<user.getid()<<" "<<user.getname()<<" "<<user.getstate()<<endl;
        }
    }
    cout<<"------------------------group list-------------------------"<<endl;
    if(!g_currentUserGroupList.empty())
    {
        for(Group &group : g_currentUserGroupList)
        {
            cout<<group.getid()<<" "<<group.getname()<<" "<<group.getdesc()<<endl;
            for(groupuser &user:group.getusers())
            {
                cout<<user.getid()<<" "<<user.getname()<<" "<<user.getstate()<<" "<<user.getrole()<<endl;
            }
        }
    }
    cout<<"==========================================================="<<endl;
}

void readTaskHandler(int cfd)
{
    while(1)
    {
        char buf[1024] = {0};
        int len = recv(cfd,buf,sizeof(buf),0);

        if(len == 0 || len == -1)
        {
            close(cfd);
            exit(0);
        }

        json js;
        js = json::parse(buf);
        int msgtype = js["msgid"];
        if(ONE_CHAT_MSG == msgtype)
        {
            cout<<js["time"]<<" id :"<<js["id"]<<"msg :"<<js["msg"]<<endl;
        }else if (GROUP_CHAT==msgtype)
        {
            cout<<"group" <<js["groupid"]<<"msg "<<js["time"]<<" id :"<<js["id"]<<"msg :"<<js["msg"]<<endl;
        }
        
    }
}

void mainMenu(int cfd)
{
    Help();
    char buffer[BUFSIZ] = {0};
    while (g_is_menu_running)
    {
        cin.getline(buffer, BUFSIZ);
        string command_buf(buffer);
        //存储命令
        string command;
        int index = command_buf.find(":");
        if (index == -1)
        {
            //help或者loginout
            command = command_buf;
        }
        else
        {
            //其他命令
            command = command_buf.substr(0, index);
        }

        auto it = command_handler_map.find(command);
        if (it == command_handler_map.end())
        {
            cerr << "invaild input command" << endl;
            continue;
        }

        //调用命令
        it->second(cfd, command_buf.substr(index + 1, command_buf.size() - index));
    }

}


void Help(int, string)
{
    cout << "--------command list--------" << endl;
    for (auto &it : command_map)
    {
        cout << it.first << " : " << it.second << endl;
    }
    cout << endl;
}

void Chat(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "chat command invaild" << endl;
    }

    int friend_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getid();
    js["name"] = g_currentUser.getname();
    js["to"] = friend_id;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error" << endl;
    }
}

void AddFriend(int clientfd, string str)
{
    int friend_id = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_FRIEND;
    js["id"] = g_currentUser.getid();
    js["friendid"] = friend_id;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addfriend msg error" << endl;
    }
}

void CreateGroup(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string group_name = str.substr(0, index);
    string group_desc = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = CREATE_GROUP;
    js["id"] = g_currentUser.getid();
    js["groupname"] = group_name;
    js["groupdesc"] = group_desc;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send creategroup msg error" << endl;
    }
}

void AddGroup(int clientfd, string str)
{
    int group_id = atoi(str.c_str());

    json js;
    js["msgid"] = GROUP_ADD_MEMBER;
    js["id"] = g_currentUser.getid();
    js["groupid"] = group_id;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addgroup msg error" << endl;
    }
}

void GroupChat(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    int group_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = GROUP_CHAT;
    js["id"] = g_currentUser.getid();
    js["name"] = g_currentUser.getname();
    js["groupid"] = group_id;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send grouochat msg error" << endl;
    }
}

void LoginOut(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGIN_OUT;
    js["id"] = g_currentUser.getid();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send grouochat msg error" << endl;
    }
    g_is_menu_running = false;
}

string getCurrentTime()
{
    return "2";
}