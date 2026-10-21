#include "result_type.h"
#include <iostream>

ResultType::ResultType(std::shared_ptr<Task> task, bool isValid)
    : task_(task), state_(std::make_shared<TaskResult>()), isValid_(isValid)
{
    // 与 Task 共享同一个控制块：信号量随 state_ 存活，
    // 不再把 ResultType 的裸指针交给 Task。
    task_->setResult(state_);
}

AnyType ResultType::GetResult() {
    std::cout << "Getting result" << std::endl;
    if (!isValid_) {
        return "Invalid result";    
    } 
    state_->semaphore_.wait();
    return std::move(state_->result_);
}

Task::Task()
    : state_(nullptr) {}

void Task::execute() {
    if (state_ != nullptr) {
        std::cout << "Receiving result" << std::endl;
        state_->result_ = run();
        state_->semaphore_.post();
    }
}

void Task::setResult(std::shared_ptr<TaskResult> state) {
    state_ = state;
}
