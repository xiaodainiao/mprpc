# mprpc
基于C++ module库 Protobuf  Zookeeper 实现的Rpc框架

## 编译：

./autobuild.sh



## 运行

bin目录下：

./provider -i test.conf  (启动Rpc提供者)

./consumer -i test.conf(启动Rpc调用者)

启动zookeeper，发现有rpc节点注册上

