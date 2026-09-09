#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <mprpc_application.h>
#include <mprpc_config.h>

// rpc服务提供者类
class RpcProvider
{
public:
    RpcProvider(muduo::net::EventLoop* loop);
    // 注册服务
    void NotifyService(google::protobuf::Service* service);
    // 启动rpc服务提供者
    void Run();

private:
    // 连接回调 
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    // 消息回调
    void OnMessage(const muduo::net::TcpConnectionPtr& conn,
                    muduo::net::Buffer* buffer,
                    muduo::Timestamp receiveTime);
    //closure回调
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,
                         google::protobuf::Message* response);

    struct ServiceInfo
    {
        google::protobuf::Service* service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> methodMap;   
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    std::unique_ptr<muduo::net::TcpServer> m_tcpServerPtr;
    muduo::net::EventLoop* m_eventLoopPtr;
};

