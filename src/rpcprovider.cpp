#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
/*
在rpcprovider类上用muduo库给它扩充了一下，让他具有一个网络服务，而且是
一个基于reactor模型的网络服务
*/

 //这里是框架提供给外部使用的，可以发布rpc方法的函数接口，所以是抽象的，不能是接收具体业务层对象，而是可以接收任意的
 void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    //获取服务的名字
    std::string service_name = pserviceDesc->name();

    //获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    //std::cout << "service_name: " << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for (int i = 0; i < methodCnt; i ++)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述），最后在框架可以通过调用它来调用应用层相应的login方法
        //UserService  Login
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
    
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        LOG_INFO("method_name:%s", method_name.c_str());
        //std::cout << "method_name: " << method_name << std::endl;
    }
    service_info.m_service = service;   //保存服务对象，到时候直接使用服务对象来调用服务方法
    m_serviceMap.insert({service_name, service_info});


    /*
    service_name: UserServiceRpc
    method_name: Login
    */
}

//启动rpc服务节点，开始提供rpc远程网络调用服务(为了方便直接将muduo库的启动都封装在了rpcserver中)                   
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str()); //atoi传char*
    muduo::net::InetAddress address(ip, port);
    
    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    /*
    远程有新的连接,TCP三次握手连接上来,muduo库会主动调用OnConnection
    就知道有新的rpc请求过来了，手法数据就用OnMessage方法进行回调
    */

    //绑定连接回调和消息读写回调方法（使用Muduo库的好处：分离了网络代码和业务代码）
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2
                                , std::placeholders::_3));

    //设置muduo库的线程数量（muduo会自动分发IO线程和工作线程），如果线程数量=1,则IO线程和工作线程就在一个线程中
    //如果设置多个线程，则一个线程只用来IO连接，生成相应的client_fd分发给工作线程,剩余的为工作线程
    server.setThreadNum(4);  //一个io线程，3个工作线程，基于reactor的网络模型（epoll）

//======================把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
   //默认会话时间是 30s  zkclient 会启动一个网络IO线程 1/3 timeout 发送Ping 消息
    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久性节点 method_name为临时节点
    for (auto &sp : m_serviceMap)
    {
        //service_name   相当于创建了/UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            //service_name/method_name 相当于创建了 /UserServiceRpc/Login 存储当前这个rpc节点服务的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            //ZOO_EPHEMERAL表示临时节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }







    std::cout << "RpcProvider start service at ip:" << ip << "port:" << port << std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();  //moudul库启动了epoll_wait以阻塞的方式来等待远程的新的连接
}

/*下面俩个回调都是Muduo库帮我们回调*/

//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        //和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name  method_name ,(UserServiceRpc  Login 你是哪个对象的哪个方法，因为rpc发布都发布成类的方法，通过服务对象进行调用)

UserServiceLoginzhang san123456  错误的，定义proto的message类型，进行数据头的序列化和反序列化
                                        service_name method_name args_size
header_size(4个字节) + header_str + args_str

std::string insert和copy方法，把整数变为二进制存储，而不是字符串，不然分不清具体长度
*/

//已建立连接用户的读写时间回调(普通方法，muduo调用时直接调用不了，因为它不是c方法，直接通过函数名调用，它是对象方法，需要进行绑定)
//如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应（图片黄色的部分序与反序）
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp)
{
    //网络上接收的远程rpc调用请求的字符流 Login  args(携带方法名和参数)
    std::string recv_buf = buffer->retrieveAllAsString();   //将字节序变为字符串

    //从字符串中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    //根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout << "rpc_header_str: " << rpc_header_str << "parse error!" << std::endl;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);


    //打印调试
    std::cout << "=============打印调试================"<< std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "====================================="<< std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap .find(service_name);

    //访问的服务不存在，比如UserService根本不存在
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist" << std::endl;
        return;
    }

    //访问的服务方法不存在，比如UserSErvice存在但是Login不存在
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exitst!" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.m_service; //获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; //获取method对象  Login

    //生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content: " << args_str << std::endl;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给下面的method方法的调用，绑定一个Closure的回调函数
   google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                   const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                        (this, &RpcProvider::SendRpcResponse, conn, response);

    //在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    //CallMethod相当于new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);

}

//Closure的回调操作，用于序列化rpc的响应和网络发送(Closure是done)
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
        
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;

    }
    conn->shutdown();   //模拟http的短链接服务，由rpcprovider主动断开连接
}

