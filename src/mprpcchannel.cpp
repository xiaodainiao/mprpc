#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "mprpcapplication.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "mprpccontroller.h"
#include "zookeeperutil.h"
/* 
哪个服务的那个方法啊（哪个服务的login方法）
    header_size + service_name method_name args_size + args
*/
//所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送    
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();//知道这个方法属于那个服务的
    std::string service_name = sd->name();  //service_name
    std::string method_name = method->name(); //method_name

    //获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;                           //当序列化成功后，序列化的数据就存在args_str中
    if (request->SerializeToString(&args_str))      //序列化将args_str内容填写到request中
    {
        args_size = args_str.size();

    }
    else
    {
        //std::cout << "serialize request error!" << std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    //把rpchead填写后，就序列化成字符串
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
       // std::cout << "serialize rpc header error!" << std::endl;
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    //组织待发送的rpc请求的字符串（按二进制存储（4个字节），不按照字符存储）
    //header_size + service_name method_name args_size + args已经获取后，就将这些字符拼接进行发送
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));  //header_size，从0写，写4个字节，写的是header_size的二进制
    send_rpc_str += rpc_header_str;  //rpcheader
    send_rpc_str += args_str;  //args

    //打印调试
    std::cout << "=============打印调试================"<< std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "====================================="<< std::endl;

    //============以上都是做rpc调用方黄色部分（序列化），下面就是做绿色部分（数据在网络中发送）
    //使用tcp编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
       
    }

    //读取配置文件rpcserver的信息
    //std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    //uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    
    
    //rpc调用方想要调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    //根据路径，在zk查一下，获取ip和port(因为rpc服务都注册到了zk上了)
   
    ZkClient zkCli;
    zkCli.Start();
    // /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    //127.0.0.1:8000
    std::string host_data = zkCli.GetDate(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }

    int idx = host_data.find(":"); //以：分割拿到ip和port
    if (idx == -1)
    {
        controller->SetFailed(method_path + "address is incalid!");
    }    
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());







    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //连接rpc服务节点（客户端连接，后面通过zookeeper来进行服务发现）
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
       // std::cout << "connect error! errno:" << errno << std::endl;
        close(clientfd);
         char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    //发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
         //std::cout << "send error! errno:" << errno << std::endl;
         close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
         return;
       
    }
    
    //接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        //std::cout << "recv error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    //反序列化rpc调用的响应数据
   // std::string response_str(recv_buf, 0, recv_size); // BUG出现问题，recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败
    //if (!response->ParseFromString(response_str))   //反序列化，将response_str填写到response中
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        //std::cout << "parse error! response_str:" << recv_buf << std::endl;
       
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }
    close(clientfd);

}