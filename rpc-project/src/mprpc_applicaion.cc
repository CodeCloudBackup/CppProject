#include "mprpc_application.h"
#include <iostream>
#include <unistd.h>

namespace mprpc
{

void ShowArgsHelp()
{
    std::cout << "Usage: ./mprpc_application [option] <config_file>" << std::endl;
    std::cout << "option:" << std::endl;
    std::cout << "  -h, --help            Show help message" << std::endl;
    std::cout << "  -i, --config-file     Specify config file" << std::endl;
}

void MprpcApplication::Init(int argc, char** argv)
{
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string configFileName;
    while ((c = getopt(argc, argv, ":i:")) != -1)
    {
        switch (c)
        {
            case 'i':
                configFileName = optarg;
                break;
            case 'h':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // 加载配置文件
    if (configFileName.empty()) {
        std::cout << "Empty config file name" << std::endl;
        return;
    }

    Config().LoadConfigFile(configFileName);
    std::cout << "rpc server ip: " << Config().LoadConfigValue("rpc_server_ip") << std::endl;
    std::cout << "rpc server port: " << Config().LoadConfigValue("rpc_server_port") << std::endl;
    std::cout << "zookeeper server ip: " << Config().LoadConfigValue("zookeeper_server_ip") << std::endl;
    std::cout << "zookeeper server port: " << Config().LoadConfigValue("zookeeper_server_port") << std::endl;
    
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication instance;
    return instance;
}

} // namespace mprpc
