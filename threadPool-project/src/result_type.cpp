#include "result_type.h"

ResultType::ResultType(std::shared_ptr<Task> task, bool isValid)
    : task_(task), isValid_(isValid)
{
    task_->setResult(this);
}

void ResultType::ReceiveResult(AnyType result) {
    this->result_ = std::move(result);
    semaphore_.post();
}

AnyType ResultType::GetResult() {
    if (!isValid_) {
        return "Invalid result";    
    } 
    semaphore_.wait();
    return std::move(result_);
}

Task::Task() 
    : result_(nullptr) {}

void Task::execute() {
    if (result_ != nullptr)
        result_->ReceiveResult(run());
}

void Task::setResult(ResultType *result) {
    result_ = result;
}
