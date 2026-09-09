#pragma once
#include <iostream>
#include <unordered_map>
#include <string>

namespace mprpc {

class MprpcConfig
{
public:
    static MprpcConfig& GetInstance(); // 获取单例实例
    void LoadConfigFile(const std::string& config_file);
    std::string LoadConfigValue(const std::string& key);
private:
    MprpcConfig();
    MprpcConfig(const MprpcConfig&) = delete;
    MprpcConfig& operator=(const MprpcConfig&) = delete;
    std::unordered_map<std::string, std::string> m_config;
};

// 使用内联函数替代宏，简短且安全
inline MprpcConfig& Config() {
    return MprpcConfig::GetInstance();
}

} // namespace mprpc