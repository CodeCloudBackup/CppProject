#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include "any_type.h"
#include "semaphore_type.h"

enum class PoolMode {
    MODE_FIXED,   // 固定数量线程
    MODE_CACHED,  // 可增长数量线程
};

class Task {
public:
    virtual AnyType run() = 0;

};

class Thread {
public:
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func) : func_(func) {}
    ~Thread();
    void start();
    void join();
    std::thread::id getThreadId() const;
private:
    ThreadFunc func_;
    std::unique_ptr<std::thread> thread_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void setMode(const PoolMode mode);  // 设置线程池模式
    void setTaskSizeThreshold(const int threshold);  // 设置任务数量阈值
    void submitTask(std::shared_ptr<Task> task);  // 提交任务
    void start(int initThreadSize = 4);  // 启动线程池，设置初始线程数量
    void stop();   // 停止线程池

   
private:
    ThreadPool(const ThreadPool&) = delete;  // 删除拷贝构造函数
    ThreadPool& operator=(const ThreadPool&) = delete;  // 删除赋值操作符

    void threadFunc();

    PoolMode poolMode_;  // 线程池模式
    size_t initThreadSize_;  // 初始线程数量
    std::vector<std::unique_ptr<Thread>> threads_;  // 线程池
    std::queue<std::shared_ptr<Task>> tasksQue_;  // 任务队列
    std::atomic<int> taskSize_;
    int taskSizeThreshold_;  // 任务数量阈值

    std::mutex tasksMutex_;         // 任务队列互斥锁   
    std::condition_variable tasksNotEmptyCV_;  // 任务队列不为空的条件变量
    std::condition_variable tasksNotFullCV_;   // 任务队列不为满的条件变量
};

#endif
