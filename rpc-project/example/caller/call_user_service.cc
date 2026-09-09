#include <iostream>
#include "mprpc_application.h"
#include <mprpc_channel.h>
#include "user.pb.h"

using namespace mprpc;

int main(int argc, char** argv)
{
    // 初始化配置文件
    MprpcApplication::Init(argc, argv);

    // 启动服务端
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // 创建请求对象
    fixbug::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123456");
    // 创建响应对象
    fixbug::LoginResponse response;
    stub.Login(nullptr, &request, &response, nullptr);
    // 处理响应结果
    if (response.result().errcode() == 0)
    {
        std::cout << "Login success: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "Login failed: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666");
    fixbug::RegisterResponse rep;
    stub.Register(nullptr, &req, &rep, nullptr);

    // 以同步的方式处理响应
    if (rep.result().errcode() == 0)
    {
        std::cout << "Register success: " << rep.success() << std::endl;
    }
    else
    {
        std::cout << "Register failed: " << rep.result().errmsg() << std::endl;
    }
    return 0;
}
