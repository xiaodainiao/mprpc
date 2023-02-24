#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

//全局的watcher观察器（回调函数） zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state,
                    const char* path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  //回调的消息类型是和会话相关的消息
    {
        if (state == ZOO_CONNECTED_STATE)  //zkserver和zkclient连接成功
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);               //给信号量sem加1，使得后面sem_wait()不阻塞
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{

}
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)   //析构时指针不为空，说明和zkserver已连接，关闭句柄
    {
        zookeeper_close(m_zhandle); //关闭句柄，释放资源
    }
}

//连接zkserver
 void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port; //zookeeper_init底层就需要用这种格式

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供三个 线程
    API调用线程
    网络I/O线程 pthread_creat poll
    watcher回调线程
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!"<< std::endl;
        return;
        //exit(ExIT_FAILURE);
    }
    //上面创建成功只是说句柄创建成功，并不是连接zkserver成功

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);  //设置上下文

    sem_wait(&sem);                 //主线程信号设置的为0，一直等待，等待sem不为0（也就是等待zkserver响应的时候，也就是回调）
    std::cout << "zookeeper_init success!" << std::endl;
}

//创建节点，state说明是永久节点还是临时节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    //先判断path表示的znode节点是否存在，如果存在，就不在重复创建
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)  //表示path的zonde节点不存在
    {
        //创建新的path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen,
                            &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success...path: " << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error...path:" << path << std::endl;
            //exit(ExIT_FAILURE);
            return ;
        }

    }
}

//根据指定的path,获取znode节点的值
std::string ZkClient::GetDate(const char* path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get znode error...path:" << path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}