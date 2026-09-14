#ifndef RESULT_TYPE_H
#define RESULT_TYPE_H

#include "any_type.h"
#include "semaphore_type.h"
#include <memory>
#include <atomic>

// Task类的前置声明
class Task;

// 接收线程池的task任务执行的返回结果
class ResultType {
public:
    ResultType(std::shared_ptr<Task> task, bool isValid = true);
    ~ResultType() = default;
    // ResultType 类包含了不可拷贝的成员emaphoreType semaphore_和std::atomic<bool> isValid_;
    void ReceiveResult(AnyType result) ;
    AnyType GetResult() ;

private:
    AnyType result_;
    SemaphoreType semaphore_;
    std::shared_ptr<Task> task_;
    std::atomic<bool> isValid_;
};

class Task {
public:
    Task();
    ~Task() = default;
    void execute() ;
    void setResult(ResultType *result);
    virtual AnyType run() = 0;
private:
    ResultType *result_; // 使用裸指针，防止与ResultType的Task循环依赖

};

#endif
