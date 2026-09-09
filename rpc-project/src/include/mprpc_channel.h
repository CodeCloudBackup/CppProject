#ifndef MPRPC_CHANNEL_H
#define MPRPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace mprpc
{

class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) override;
};

}

#endif
