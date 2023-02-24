#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>   //映射表，用来存储服务对象和服务方法的映射

//框架提供的专门服务发布rpc服务的网络对象类，可以接收所有类，而不仅仅是UserService
//使用moudle库
class RpcProvider
{
public:

    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口，所以是抽象的，不能是接收具体业务层对象，而是可以接收任意的
    //void NotifyService(google::protobuf::Service *service){}   方法的声明，实现写在cpp文件中
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    //void Run(){}                                               方法的声明，实现写在cpp文件中
    void Run();
private:
    //组合了TcpServer
   // std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;

    //组合了EventLoop(epoll)
    muduo::net::EventLoop m_eventLoop;

    //RpcProvider中的NotifyService
    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;   //保存服务对象
        std::unordered_map<std::string, const::google::protobuf::MethodDescriptor*> m_methodMap;  //保存服务方法
    };

    //存储注册成功的服务对象和其服务方法的所有信息（服务名字对应服务信息，服务信息中有具体的服务对象和服务的所有方法）
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);

    //已建立连接用户的读写时间回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    //Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};