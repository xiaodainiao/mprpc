set(SRC_LIST calluserservice.cpp ../user.pb.cc)
#set(SRC_LIST callfriendservice.cpp ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})  #providere编译是需要用到框架的代码，所以动态链接src库
target_link_libraries(consumer mprpc protobuf) # provider库依赖了mprpc和protobuf