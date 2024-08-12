#ifndef PUBLIC_H
#define PUBLIC_H

/*
服务器和客户端共有内容
*/

enum enMsgtype
{
    LOGIN_MSG = 1,
    LOGIN_MSG_ACK = 2,
    REG_MSG = 3,
    REG_MSG_ACK = 4,
    ONE_CHAT_MSG = 5,
    ADD_FRIEND = 6,
    CREATE_GROUP = 7,
    GROUP_ADD_MEMBER = 8,
    GROUP_CHAT = 9,
    LOGIN_OUT=10
};

#endif