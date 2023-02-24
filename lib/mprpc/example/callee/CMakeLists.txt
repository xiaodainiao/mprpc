set(SRC_LIST userservice.cpp ../user.pb.cc)
#set(SRC_LIST friendservice.cpp ../friend.pb.cc)

add_executable(provider ${SRC_LIST})  #providere编译是需要用到框架的代码，所以动态链接src库
target_link_libraries(provider mprpc protobuf) # provider库依赖了mprpc和protobuf