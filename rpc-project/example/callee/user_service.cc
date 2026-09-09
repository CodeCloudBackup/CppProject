#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpc_application.h"
#include "mprpc_config.h"
#include "mprpc_provider.h"

using namespace mprpc;

class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "Received Login request"<< std::endl;
        std::cout << "name: " << name << ", pwd: " << pwd << std::endl;
        return true;
    }

    void Login(::google::protobuf::RpcController* controller,
              const ::fixbug::LoginRequest* request,
              ::fixbug::LoginResponse* response,
              ::google::protobuf::Closure* done)
    {
        // 模拟业务处理
        std::string uname = request->name();
        std::string pwd = request->pwd();

        // 业务逻辑处理
        bool login_result = Login(uname, pwd);
        
        fixbug::ResultCode* result_code = response->mutable_result();
        result_code->set_errcode(0);
        result_code->set_errmsg("");

        response->set_success(login_result);
        // 处理响应
        done->Run();
    }

    bool Register(uint32_t id, std::string name, std::string pwd) 
    {
        std::cout << "Received Register request" << std::endl;
        std::cout << "id:"<< id << ", name: " << name << ", pwd: " << pwd << std::endl;
        return true;
    }

    void Register(::google::protobuf::RpcController* controller,
              const ::fixbug::RegisterRequest* request,
              ::fixbug::RegisterResponse* response,
              ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char** argv)
{
    MprpcApplication::Init(argc, argv);
    muduo::net::EventLoop loop; // 1. 主线程创建唯一的 EventLoop
    RpcProvider provider(&loop);
    provider.NotifyService(new UserService());
    provider.Run();
    
    return 0;
}