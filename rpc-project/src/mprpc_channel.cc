#include "mprpc_channel.h"
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include "mprpc_application.h"
#include "mprpc_controller.h"
#include "zookeeper_util.h"

using namespace std;

namespace mprpc
{
    // 辅助函数：将 errno 描述拼接进错误信息
    static void SetFailedWithErrno(google::protobuf::RpcController* controller,
                                   const char* prefix) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s%s", prefix, strerror(errno));
        controller->SetFailed(buf);
    }

    void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                  google::protobuf::RpcController* controller,
                                  const google::protobuf::Message* request,
                                  google::protobuf::Message* response,
                                  google::protobuf::Closure* done)
    {
        const google::protobuf::ServiceDescriptor* serviceDesc = method->service();
        const string serviceName = serviceDesc->name();
        const string methodName = method->name();
        string argsStr;
        if(!request->SerializeToString(&argsStr))
        {
            controller->SetFailed("request serialize fail");
            return;
        }
        uint32_t argsSize = argsStr.size();
        // 设置rpcHeader
        mprpc::RpcHeader rpcHeader;
        rpcHeader.set_service_name(serviceName);
        rpcHeader.set_method_name(methodName);
        rpcHeader.set_args_size(argsSize);

        string rpcHeaderStr;
        if(!rpcHeader.SerializeToString(&rpcHeaderStr))
        {
            controller->SetFailed("rpcHeader serialize fail");
            return;
        }
        uint32_t rpcHeaderSize = rpcHeaderStr.size();

        string sendStr;
        sendStr.insert(0, std::string((char*)&rpcHeaderSize, sizeof(uint32_t)));
        sendStr += rpcHeaderStr;
        sendStr += argsStr;

        cout << "==============================="<< endl;
        cout << "header size  :" << rpcHeaderSize << endl;
        cout << "header str   :" << rpcHeaderStr << endl;
        cout << "service name :" << serviceName << endl;
        cout << "method name  :" << methodName << endl;
        cout << "args str     :" << argsStr << endl;
        cout << "==============================="<< endl;
        cout << "sendStr: " << sendStr << endl;

        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        if (clientfd < 0) {
            cout << "socket create fail! errno:" << strerror(errno) << endl;
            SetFailedWithErrno(controller, "socket create fail! errno:");
            return;
        }

        //std::string ip = Config().LoadConfigValue("rpc_server_ip");
        //std::string port =  Config().LoadConfigValue("rpc_server_port");
        ZkClient zkCli;
        zkCli.Start();
        const string methodPath = "/" + serviceName + "/" + methodName;
        const string hostData = zkCli.GetData(methodPath);

        if (hostData.empty()) {
            controller->SetFailed("host data is empty");
            return;
        }
        int idx = hostData.find(":");
        if (idx == -1) {
            controller->SetFailed("invalid host data");
            return;
        }
        const string ip = hostData.substr(0, idx);
        const uint16_t port = stoi(hostData.substr(idx + 1, hostData.size() - idx));
        cout << "ip:" << ip << " port:" << port << endl;

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        if (connect(clientfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            cout << "connect fail! errno:" << strerror(errno) << endl;
            close(clientfd);
            SetFailedWithErrno(controller, "connect fail! errno:");
            return;
        }
        if (send(clientfd, sendStr.c_str(), sendStr.size(), 0) < 0) {
            SetFailedWithErrno(controller, "send fail! errno:");
            close(clientfd);
            return;
        }

        char recvBuf[1024] = {0};
        int recvSize = 0;
        if((recvSize = recv(clientfd, recvBuf, sizeof(recvBuf), 0)) < 0) {
            SetFailedWithErrno(controller, "recv fail! errno:");
            close(clientfd);
            return;
        }
        if (!response->ParseFromArray(recvBuf, recvSize)) {
            controller->SetFailed("response parse fail");
            close(clientfd);
            return;
        }        
        close(clientfd);
    }
} // namespace mprpc
