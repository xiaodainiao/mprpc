#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"


class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service! userid:" << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("ga ga");
        vec.push_back("ji ji");
        vec.push_back("gu gu");
        return vec;
    }

    //重写基类方法
     void GetFriendsList(::google::protobuf::RpcController* controller, 
                    const ::fixbug::GetFriendsListRequest* request,
                    ::fixbug::GetFriendsListResponse* response,
                    ::google::protobuf::Closure* done)
        {
            uint32_t userid = request->userid();
            std::vector<std::string> friendsList = GetFriendsList(userid);
            response->mutable_result()->set_errcode(0);
            response->mutable_result()->set_errmsg("");
            for (std::string &name : friendsList)
            {
                std::string *p = response->add_friends();
                *p = name;        //把response中的friends信息就修改成从vector中拿出的姓名
            }
            done->Run();
        }

};
int main(int argc, char **argv)
{
    /*测试一下日志功能
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    */

    //调用框架的初始化操作（相应配置，rpc的ip和端口号，zookeeper的ip和端口号）
    //通过读取相应的配置文件，不能写死
    // provider -i config.conf
    MprpcApplication::Init(argc, argv);

    //把UserService对象发布到rpc节点上（不仅可以发布UserService，还可以发布商品等）
    RpcProvider provider;    //定义一个可以发布服务的对象，在对象上发布服务，provider实际上是一个rpc网络服务对象（提供什么服务呢？提供rpc方法）
    provider.NotifyService(new FriendService());
    //protobuf.NotifyService(new ProductService);


    //启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}
