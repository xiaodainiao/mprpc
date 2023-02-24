#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
using namespace fixbug;
/*
UserServiceRpc原来是一个本地服务，提供了俩个进程内的本地方法，Login和GetFriendLists
*/
class UserService : public UserServiceRpc //使用在rpc服务发布端(rpc服务提供者)
{
    public:
        bool Login(std::string name, std::string pwd)
        {
            std::cout << "doing local service:Login" << std::endl;
            std::cout << "name:" << name << "pwd:" << pwd << std::endl;
            return true;
        }


         bool Register(uint32_t id, std::string name, std::string pwd)
        {
            std::cout << "doing local service:Register" << std::endl;
            std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
            return true;
        }
    /*
    重新基类UserServiceRpc的虚函数
    1. caller ===>  Login(LoginRequest)  => muduo =>  callee   //rpc调用者想要发起rpc请求
    2. cllee  ===>  Login(LoginRequest)  => 交到你重写的这个Login方法上了
    */
        void Login(::google::protobuf::RpcController* controller, 
                    const ::fixbug::LoginRequest* request,
                    ::fixbug::LoginResponse* response,
                    ::google::protobuf::Closure* done)
        {
            //1.框架给业务上报了请求参数LoginRequest, 应用获取相应数据做本地业务
            std::string name = request->name();
            std::string pwd = request->pwd();

            //2.做本地业务
            bool login_result = Login(name, pwd);

            //3.把响应写入  包括错误码、错误消息、返回值
            ResultCode * code = response->mutable_result();
            code->set_errcode(0);
            code->set_errmsg("");
            response->set_sucess(login_result);

            //4.执行回调操作  执行响应对象数据的序列化和网络发送（都是由框架完成的）
            done->Run();
        }

         void Register(::google::protobuf::RpcController* controller, 
                        const ::fixbug::RegisterRequest* request,
                        ::fixbug::RegisterResponse* response,
                        ::google::protobuf::Closure* done)
         {
            uint32_t id = request->id();
            std::string name = request->name();
            std::string pwd = request->pwd();

            bool ret = Register(id, name, pwd);
            response->mutable_result()->set_errcode(0);
            response->mutable_result()->set_errmsg("");
            response->set_sucess(ret);

            done->Run();   //这个回调是框架做的，把填好的response进行数据的序列化并通过网络发回到rpcclient
         }
};      


int main(int argc, char **argv)
{
    //调用框架的初始化操作（相应配置，rpc的ip和端口号，zookeeper的ip和端口号）
    //通过读取相应的配置文件，不能写死
    // provider -i config.conf
    MprpcApplication::Init(argc, argv);

    //把UserService对象发布到rpc节点上（不仅可以发布UserService，还可以发布商品等）
    RpcProvider provider;    //定义一个可以发布服务的对象，在对象上发布服务，provider实际上是一个rpc网络服务对象（提供什么服务呢？提供rpc方法）
    provider.NotifyService(new UserService());
    //protobuf.NotifyService(new ProductService);


    //启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}