#ifndef MPRPC_CONTROLLER_H
#define MPRPC_CONTROLLER_H
#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset() override ;
    bool Failed() const override ;
    std::string ErrorText() const ;
    void SetFailed(const std::string& reason) override;

    // 暂不需实现
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback) override;

private:
    bool m_failed;
    std::string m_errText;
};


#endif
