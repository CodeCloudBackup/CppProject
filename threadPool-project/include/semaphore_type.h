#ifndef SEMAPHORE_TYPE_H
#define SEMAPHORE_TYPE_H

#include <mutex>
#include <condition_variable>


class SemaphoreType {
public:
    SemaphoreType(int count = 0) : count_(count) {}
    ~SemaphoreType() = default;
    // 获取一个信号量
    void wait(){
        std::unique_lock<std::mutex> lock(mutex_);
        --count_;
        if (count_ < 0) {
            cond_.wait(lock);
        }
    }
    // 释放一个信号量
    void post() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
        if( count_ <= 0) cond_.notify_one();
    }
    void notify();
    SemaphoreType(const SemaphoreType&) = delete;
    SemaphoreType& operator=(const SemaphoreType&) = delete;
private:
    int count_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif
