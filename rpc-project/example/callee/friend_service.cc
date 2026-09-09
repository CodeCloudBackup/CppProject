#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "mprpc_application.h"
#include "mprpc_provider.h"

using namespace mprpc;

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t userid)
    {
        std::cout << "FriendService GetFriedList userid: " << userid << std::endl;
        return std::vector<std::string>{"zhang3", "li4", "wang5"};
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friend_list = GetFriendList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string& name : friend_list)
        {
            response->add_friends(name);
        }
        done->Run();
    }
};

int main(int argc, char** argv)
{
    MprpcApplication::Init(argc, argv);
    muduo::net::EventLoop loop; // 1. 主线程创建唯一的 EventLoop
    RpcProvider provider(&loop);
    provider.NotifyService(new FriendService());
    provider.Run();
    
    return 0;
}