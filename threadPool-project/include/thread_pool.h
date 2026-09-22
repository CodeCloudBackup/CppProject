#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include "any_type.h"
#include "result_type.h"
#include "semaphore_type.h"

enum class PoolMode {
    MODE_FIXED,   // 固定数量线程
    MODE_CACHED,  // 可增长数量线程
};



class Thread {
public:
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func) : func_(func), threadId_(generateId_++) {}
    ~Thread(){};
    void start();
    int getId() const { return threadId_; }
    
private:
    ThreadFunc func_;
    static int  generateId_;
    int threadId_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;  // 删除拷贝构造函数
    ThreadPool& operator=(const ThreadPool&) = delete;  // 删除赋值操作符
    void setMode(const PoolMode mode);  // 设置线程池模式
    void setTaskSizeThreshold(const int threshold);  // 设置任务数量阈值
    void setThreadSizeThreshold(const int threshold);  // 设置线程数量阈值
    ResultType submitTask(std::shared_ptr<Task> task);  // 提交任务
    void start(int initThreadSize = std::thread::hardware_concurrency());  // 启动线程池，设置初始线程数量
    void stop();   // 停止线程池

   
private:

    void threadFunc(const int threadId);
    bool CheckPoolRunning() const { return isPoolRunning_; }

    bool isPoolRunning_ ;
    PoolMode poolMode_;                 // 线程池模式
    size_t initThreadSize_;             // 初始线程数量
    std::atomic<int> idleThreadSize_;   // 空闲线程数量
    size_t threadSizeThreshold_;        // 线程数量阈值
    std::atomic<int> curThreadSize_;       // 线程数量

    // key=threadId, value=std::unique_ptr<Thread>
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;  // 线程池
    std::queue<std::shared_ptr<Task>> tasksQue_;  // 任务队列
    std::atomic<int> taskSize_;
    int taskSizeThreshold_;  // 任务数量阈值

    std::mutex tasksMutex_;         // 任务队列互斥锁   
    std::condition_variable tasksNotEmptyCV_;  // 任务队列不为空的条件变量
    std::condition_variable tasksNotFullCV_;   // 任务队列不为满的条件变量
    std::condition_variable exitCodeCV_;   // 线程退出码条件变量
};

#endif
