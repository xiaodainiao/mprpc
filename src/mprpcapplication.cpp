#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>
MprpcConfig MprpcApplication::m_config;
void showArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}
//类外实现静态方法不用static
void MprpcApplication::Init(int argc, char **argv)    //只负责配置文件的初始化，配置文件的读取不要在这里面写，分模块写
{
    if (argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch(c)
        {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                showArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                showArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    //开始加载配置文件了（专门在一个.h和.cpp文件写配置文件）
    //rpcserver_ip   rpcserver_port   zookeeper_ip   zookeeper_port
    m_config.LoadConfigFile(config_file.c_str());
   /*  std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl; */
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}