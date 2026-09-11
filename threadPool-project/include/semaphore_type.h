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
        cond_.wait(lock, [&]()->bool{ return count_ > 0; });
        count_--;
    }
    // 释放一个信号量
    void post() {
        std::unique_lock<std::mutex> lock(mutex_);
        count_++;
        cond_.notify_all();
    }
    void notify();
private:
    int count_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif
