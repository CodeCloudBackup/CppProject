#include "mprpc_config.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace mprpc
{
    
MprpcConfig::MprpcConfig() 
{
}

MprpcConfig& MprpcConfig::GetInstance()
{
    static MprpcConfig instance;
    return instance;
}


void MprpcConfig::LoadConfigFile(const std::string& config_file)
{
    std::ifstream conf_file(config_file);
    if (!conf_file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (getline(conf_file, line))
    {
        // 1. 去除行首和行尾的空格（防止配置写成 "  key = value  " 导致解析失败）
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue; // 全空行，跳过
        size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);

        // 2. 忽略注释和空行
        if (line[0] == '#' || line.empty()) continue;

        // 3. 查找 '=' 的位置
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue; // 没有等号，不是合法的键值对，跳过

        // 4. 截取 key 和 value，并顺便去除它们首尾的空格
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // 去除 key 尾部的空格
        size_t key_end = key.find_last_not_of(" \t");
        if (key_end != std::string::npos) key = key.substr(0, key_end + 1);

        // 去除 value 首部的空格
        size_t val_start = value.find_first_not_of(" \t");
        if (val_start != std::string::npos) value = value.substr(val_start);

        // 5. 存入 map
        m_config[key] = value;
    }
    conf_file.close();
}

std::string MprpcConfig::LoadConfigValue(const std::string& key)
{

    if (m_config.find(key) != m_config.end()) {
        return m_config[key];
    } else {
        return "";
    }
}

} // namespace mprpc