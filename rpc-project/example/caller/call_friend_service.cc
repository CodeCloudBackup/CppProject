#include <iostream>
#include "mprpc_application.h"
#include "friend.pb.h"

using namespace mprpc;

int main(int argc, char** argv)
{
    // 初始化配置文件
    MprpcApplication::Init(argc, argv);

    // 启动服务端
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    // 创建请求对象
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    // 创建响应对象
    fixbug::GetFriendListResponse response;
    MprpcController controller;
    stub.GetFriendList(&controller, &request, &response, nullptr);

    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return -1;
    }
    // 处理响应结果
    if (response.result().errcode() == 0)
    {
        std::cout << "GetFriendList success: " << response.friends_size() << std::endl;
        int size = response.friends_size();
        for (int i = 0; i < size; ++i)
        {
            std::cout << "index" << i+1 << ", friend: " << response.friends(i) << std::endl;
        }
    }
    else
    {
        std::cout << "GetFriendList failed: " << response.result().errmsg() << std::endl;
    }
    return 0;
}