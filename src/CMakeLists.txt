#aux_source_directory(. SRC_LIST)  #当前目录下所有源文件名字放入SRC_LIST
# set(SRC_LIST mprpcapplication.cpp mprpcconfig.cpp)
#add_library(mprpc SHARED ${SRC_LIST})  #生成动态库
set(SRC_LIST 
    mprpcapplication.cpp 
    mprpcconfig.cpp 
    rpcheader.pb.cc 
    rpcprovider.cpp 
    mprpcchannel.cpp
    mprpccontroller.cpp
    logger.cpp
    zookeeperutil.cpp)
add_library(mprpc ${SRC_LIST})  #生成静态库
target_link_libraries(mprpc muduo_net muduo_base   pthread zookeeper_mt) #mprpc依赖于muduo库

#muduo库是静态库，现在需要用动态库链接静态库，而且muduo_net和muduo_base之间相互有联系