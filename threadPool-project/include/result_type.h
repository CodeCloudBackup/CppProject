#ifndef RESULT_TYPE_H
#define RESULT_TYPE_H

#include "any_type.h"
#include "semaphore_type.h"
#include <memory>
#include <atomic>

// Task类的前置声明
class Task;

// 结果与同步信号量的共享控制块。
// 由 ResultType 与 Task 共同以 shared_ptr 持有：
// 即便 ResultType 先于工作线程析构，Task 仍能安全地向一个存活的
// SemaphoreType 执行 post()，避免在已销毁的对象上 notify_one 而死锁。
struct TaskResult {
    AnyType result_;
    SemaphoreType semaphore_;
};

// 接收线程池的task任务执行的返回结果
class ResultType {
public:
    ResultType(std::shared_ptr<Task> task, bool isValid = true);
    ~ResultType() = default;
    // ResultType 类包含了不可拷贝的成员 SemaphoreType 和 std::atomic<bool> isValid_;
    AnyType GetResult() ;

private:
    std::shared_ptr<Task> task_;
    std::shared_ptr<TaskResult> state_;
    std::atomic<bool> isValid_;
};

class Task {
public:
    Task();
    ~Task() = default;
    void execute() ;
    void setResult(std::shared_ptr<TaskResult> state);
    virtual AnyType run() = 0;
private:
    // 共享所有权的结果控制块，保证信号量生命周期不短于任务执行
    std::shared_ptr<TaskResult> state_;

};

#endif
