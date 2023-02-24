# mprpc
基于C++ muduo库 Protobuf  Zookeeper 实现的Rpc框架

## 编译：

`./autobuild.sh`



## 运行

bin目录下：

`./provider -i test.conf ` (启动Rpc提供者)

`./consumer -i test.conf`(启动Rpc调用者)

启动zookeeper，发现有rpc节点注册上



# 技术选型

- Protobuf
- Zookeeper
- C++
- Module库



## 目录结构

```c=+
.
├── a.out
├── autobuild.sh
├── bin
│   ├── 2023-2-20-log.txt
│   ├── 2023-2-24-log.txt
│   ├── consumer
│   ├── provider
│   └── test.conf
├── build
├── c++
│   └── test.cpp
├── CMakeLists.txt
├── example
│   ├── callee
│   │   ├── CMakeLists.txt
│   │   ├── friendservice.cpp
│   │   └── userservice.cpp
│   ├── caller
│   │   ├── callfriendservice.cpp
│   │   ├── calluserservice.cpp
│   │   └── CMakeLists.txt
│   ├── CMakeLists.txt
│   ├── friend.pb.cc
│   ├── friend.pb.h
│   ├── friend.proto
│   ├── user.pb.cc
│   ├── user.pb.h
│   └── user.proto
├── lib
├── README.md
├── src
│   ├── CMakeLists.txt
│   ├── include
│   │   ├── lockqueue.h
│   │   ├── logger.h
│   │   ├── mprpcapplication.h
│   │   ├── mprpcchannel.h
│   │   ├── mprpcconfig.h
│   │   ├── mprpccontroller.h
│   │   ├── rpcheader.pb.h
│   │   ├── rpcprovider.h
│   │   └── zookeeperutil.h
│   ├── logger.cpp
│   ├── mprpcapplication.cpp
│   ├── mprpcchannel.cpp
│   ├── mprpcconfig.cpp
│   ├── mprpccontroller.cpp
│   ├── rpcheader.pb.cc
│   ├── rpcheader.proto
│   ├── rpcprovider.cpp
│   └── zookeeperutil.cpp
├── test
└── test.cpp

```



- `autobuild.sh `：启动脚本
- `bin`：可执行文件
- `build`：CMake构建编译
- `example`使用框架（RPC服务的提供者和消费者）
  - `callee`：提供RPC服务（服务端）
  - `caller`：调用RPC服务（客户端）
- `lib`：编译成动态库
- `src`：框架源码(主要包含下面)
  - `rpcprovider.cpp`：rpc提供者
  - `mprpcrpcchannel.cpp`：rpc的channel
  - `logger.cpp`：日志
  - `zookeeperutil.cpp`：zookeeper
- `test`：测试用例

## 架构设计

![image-20230224165314848](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230224165314848.png)
