#include "mprpc_provider.h"
#include "zookeeper_util.h"
#include "rpcheader.pb.h"
#include "logger.h"

RpcProvider::RpcProvider(muduo::net::EventLoop* loop)
: m_eventLoopPtr(loop)
{
}

void RpcProvider::NotifyService(google::protobuf::Service* service)
{
    ServiceInfo serviceInfo;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* serviceDesc = service->GetDescriptor();
    // 获取服务对象的名称
    std::string serviceName = serviceDesc->name();
    int methodCount = serviceDesc->method_count();
    LOG_INFO("service name: %s", serviceName.c_str());
    LOG_INFO("method count: %d", methodCount);
    for (int i = 0; i < methodCount; i++)
    {
        const google::protobuf::MethodDescriptor* methodDesc = serviceDesc->method(i);
        std::string methodName = methodDesc->name();
        LOG_INFO("method name: %s", methodName.c_str());
        serviceInfo.methodMap.insert({methodName, methodDesc});
    }
    serviceInfo.service = service;
    m_serviceMap.insert({serviceName, serviceInfo});
}

void RpcProvider::Run()
{
    const std::string ip = mprpc::Config().LoadConfigValue("rpc_server_ip");
    const std::string portStr = mprpc::Config().LoadConfigValue("rpc_server_port");
    if (ip.empty() || portStr.empty()) {
        LOG_ERROR("rpc_server_ip or rpc_server_port is not configured!");
        return;
    }
    uint16_t port = static_cast<uint16_t>(std::atoi(portStr.c_str()));
    LOG_INFO("RpcProvider start service at %s:%s", ip, portStr);
    muduo::net::InetAddress address(ip, port);
    m_tcpServerPtr = std::make_unique<muduo::net::TcpServer>(
        m_eventLoopPtr, 
        address, 
        "RpcProvider"
    );
    //muduo::net::TcpServer server(m_eventLoopPtr, address, "RpcProvider");
    // 绑定回调函数
    m_tcpServerPtr->setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    
    m_tcpServerPtr->setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
        std::placeholders::_2, std::placeholders::_3));
    
    m_tcpServerPtr->setThreadNum(4);
    ZkClient zkCli;
    zkCli.Start();
    for (auto& serviceInfo : m_serviceMap)
    {
        // /service_name/method_name
        std::string servicePath = "/" + serviceInfo.first;
        zkCli.Create(servicePath, "", 0); 
        for (auto& methodInfo : serviceInfo.second.methodMap)
        {
            std::string methodName = methodInfo.first;
            std::string methodPath = servicePath + "/" + methodName;
            std::string methodData = ip + ":" + portStr;
            zkCli.Create(methodPath, methodData, ZOO_EPHEMERAL);
        }
    }
    m_tcpServerPtr->start();
    m_eventLoopPtr->loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer* buffer,
                            muduo::Timestamp receiveTime)
{
    std::string recvBuf = buffer->retrieveAllAsString();

    uint32_t headerSize = 0;
    recvBuf.copy((char*)&headerSize, sizeof(uint32_t), 0);

    std::string rpcHeaderStr = recvBuf.substr(4, headerSize);
    mprpc::RpcHeader rpcHeader;
    if (!rpcHeader.ParseFromString(rpcHeaderStr)) {
        LOG_ERROR("header str %s", rpcHeaderStr);
        LOG_ERROR("rpcHeader.ParseFromString fail");
        return;
    }

    std::string serviceName = rpcHeader.service_name();
    std::string methodName = rpcHeader.method_name();
    uint32_t argSize = rpcHeader.args_size();
    std::string argsStr = recvBuf.substr(4 + headerSize, argSize);
    LOG_INFO("====================================");
    LOG_INFO("rpcHeader.ParseFromString success");
    LOG_INFO("header size  :%d", headerSize);
    LOG_INFO("header str   :%s", rpcHeaderStr);
    LOG_INFO("service name :%s", serviceName);
    LOG_INFO("method name  :%s", methodName);
    LOG_INFO("arg size     :%d", argSize);
    LOG_INFO("arg str      :%s", argsStr);
    LOG_INFO("====================================");

    // 获取服务对象
    auto serviceIt = m_serviceMap.find(serviceName);
    if (serviceIt == m_serviceMap.end()) {
        LOG_ERROR("service %s not found", serviceName);
        return;
    }
        // 获取服务对象
    ServiceInfo& serviceInfo = serviceIt->second;
    // 获取方法对象
    auto methodIt = serviceInfo.methodMap.find(methodName);
    if (methodIt == serviceInfo.methodMap.end()) {
        LOG_ERROR("method %s not found", methodName);
        return;
    }
    google::protobuf::Service* service = serviceInfo.service;
    const google::protobuf::MethodDescriptor* methodDesc = methodIt->second;
    // 创建请求对象和响应对象
    google::protobuf::Message* request = service->GetRequestPrototype(methodDesc).New();
    // 解析请求参数
    if (!request->ParseFromString(argsStr)) {
        LOG_ERROR("request parse fail");
        return;
        
    } 
    google::protobuf::Message* response = service->GetResponsePrototype(methodDesc).New();
    
    google::protobuf::Closure* done = google::protobuf::NewCallback
        <RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(
                this, &RpcProvider::SendRpcResponse, conn, response);
    // 调用服务对象的method方法
    serviceInfo.service->CallMethod(methodDesc, nullptr, request, response, done);
    // 获取响应结果
    std::string responseStr = response->SerializeAsString();
    // 发送响应结果
    conn->send(responseStr);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string responseStr;
    if(!response->SerializeToString(&responseStr)) {
        LOG_ERROR("response serialize fail");
        return;
    }
    conn->send(responseStr);
    conn->shutdown();
}
