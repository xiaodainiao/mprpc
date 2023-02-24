#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc, char **argv)
{
    //整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    //演示调用远程发布的rpc方法login
    //fixbug::UserService_Stub stub(RpcChannel); RpcChannel不能直接使用，因为RpcChannel是一个抽象类不能new一个对象（callMethod是一个纯虚函数）
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    //rpc方法的请求参数，这是rpc调用方传递的，而rpc response是rpc提供端封装返回的
    fixbug::LoginRequest request;
    request.set_name("xiao dai niao");
    request.set_pwd("123456");

    //rpc方法的响应
    fixbug::LoginResponse response;

    //发起rpc方法调用，同步rpc调用过程 MprpcChannel::callMethod
    stub.Login(nullptr, &request, &response, nullptr);
                    //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
                   //不管stub有什么方法（login,登录，注册，列表等最终都是调用的callMethod方法）
   
    //一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << response.sucess() << std::endl;

    }
    else
    {
        std::cout << "rpc login response error" << response.result().errcode() << std::endl;
    }

    //演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("66666");
    fixbug::RegisterResponse rsp;

    //以同步的阻塞的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);

    //一次rpc调用完成，读调用结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc register response success:" << response.sucess() << std::endl;

    }
    else
    {
        std::cout << "rpc register response error" << response.result().errmsg() << std::endl;
    }
    return 0;
}