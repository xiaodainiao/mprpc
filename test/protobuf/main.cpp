#include "test.pb.h"
#include <iostream>
#include <string>
#include <typeinfo>
using namespace fixbug;

int main()
{
    /* LoginResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登录失败") */

    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list();     //往列表添加一个好友
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAX);

     User *user2 = rsp.add_friend_list();     //往列表添加俩个好友
    user2->set_name("li si");
    user2->set_age(20);
    user2->set_sex(User::MAX);

    std::cout << rsp.friend_list_size() << std::endl;

    User us = rsp.friend_list(1);
     std::cout << us.name() << std::endl;
}

int main1()
{
    //封装login请求对象的数据
    LoginRequest req;
    req.set_name("xiaodainiao");
    req.set_pwd("666666");

    //对象序列化
    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    //反序列化
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
    return 0;
}