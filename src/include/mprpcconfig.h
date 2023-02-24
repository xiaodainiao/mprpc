# pragma once
//键值对，读取后把它放进map表里，供框架其他模块很轻松就读取配置文件
//不需要每执行一个软件就读一次配置文件，一次性把配置文件加载完成
#include <unordered_map>
#include <string>

class MprpcConfig
{
    public:
        //负责解析加载配置文件
        void LoadConfigFile(const char *config_file);
        //查询配置项信息

        std::string Load(const std::string &key);
    private:
        std::unordered_map<std::string, std::string> m_configMap;

        //去掉字符串前后的空格
        void Trim(std::string &src_buf);
};