#ifndef MPRPC_APPLICATION_H
#define MPRPC_APPLICATION_H

#include "mprpc_channel.h"
#include "mprpc_config.h"
#include "mprpc_controller.h"


namespace mprpc
{
// mprpc框架的基础类
class MprpcApplication
{
public:
    static void Init(int argc, char** argv);
    static MprpcApplication& GetInstance();
    static std::string GetConfigValue(const std::string& key);

private:
    MprpcApplication() {}
    ~MprpcApplication() {}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
    MprpcApplication& operator=(MprpcApplication&&) = delete;

    static MprpcApplication* m_instance;
    static std::string m_config_file;
};

} // namespace mprpc

#endif
