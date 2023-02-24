#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
//mprpc框架的基础类，负责框架的一些初始化
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();

private:
    static MprpcConfig m_config;   //静态函数不能访问普通变量,类的静态成员函数在类外初始化
  
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;  //把有关拷贝构造的都删掉
};