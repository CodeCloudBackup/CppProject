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
#include <thread>
#include <future>
#include <chrono>

#include "semaphore_type.h"

const int TASK_SIZE_THRESHOLD = 3;
const int THREAD_MAX_THRESHOLD = 2;
const int THREAD_MAX_IDLE_TIME = 60;

enum class PoolMode {
    MODE_FIXED,   // 固定数量线程
    MODE_CACHED,  // 可增长数量线程
};

class Thread {
public:
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func) : func_(func), threadId_(generateId_++) {}
    ~Thread(){};
    void start()
    {   
        std::thread t(func_, threadId_);
        t.detach();
    }
    int getId() const { return threadId_; }
    
private:
    ThreadFunc func_;
    static int  generateId_;
    int threadId_;
};

int Thread::generateId_ = 0;

/**
 * @brief 线程池类
 * 
 */
class ThreadPool {
public:
    ThreadPool() 
       :initThreadSize_(0), 
        taskSize_(0),
        taskSizeThreshold_(TASK_SIZE_THRESHOLD), 
        threadSizeThreshold_(128),
        poolMode_(PoolMode::MODE_FIXED),
        isPoolRunning_(false),
        idleThreadSize_(0),
        curThreadSize_(0)
    {
    }

    ~ThreadPool()
    {
        isPoolRunning_ = false;
        std::unique_lock<std::mutex> lock(tasksMutex_);
        tasksNotEmptyCV_.notify_all();
        exitCodeCV_.wait(lock, [&]()->bool { return tasksQue_.size() == 0; });
    }

    // 设置线程池模式
    void setMode(const PoolMode mode) 
    {
        if (CheckPoolRunning()) {
            std::cerr << "Pool is running, cannot set mode" << std::endl;
            return;
        }
        poolMode_ = mode;
    }
     // 设置任务数量阈值
    void setTaskSizeThreshold(const int threshold) 
    {
        taskSizeThreshold_ = threshold;
    }
    // 设置线程数量阈值
    void setThreadSizeThreshold(const int threshold) 
    {
        if (CheckPoolRunning()) {
            std::cerr << "Pool is running, cannot set thread size threshold" << std::endl;
            return;
        }
        if (poolMode_ == PoolMode::MODE_CACHED) {
            threadSizeThreshold_ = threshold;
        } else {
            std::cerr << "Cannot set thread size threshold in fixed mode" << std::endl;
        }
    }
    // 提交任务
    // 使用可变参模板编程
    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        using RetType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RetType> ResultType = task->get_future();
        // 获取锁
        std::unique_lock<std::mutex> lock(tasksMutex_);
        // 等待任务队列有空闲位置
        // 用户指定等待时间，最长时间不能阻塞1s，否者判断任务提交失败
        if(!tasksNotFullCV_.wait_for(lock, std::chrono::seconds(1), 
            [&]()->bool { return tasksQue_.size() < (size_t)taskSizeThreshold_; }))
        {
            std::cerr << "Task queue is full, submit task timeout" << std::endl;
            auto task = std::make_shared<std::packaged_task<RetType()>> (
                []()->RetType { return RetType(); });
            (*task)();
            return task->get_future();
        }
        // 将任务放入任务队列
        tasksQue_.emplace([task](){ (*task)();});
        taskSize_++;
        std::cout << "Task submitted, task size: " << taskSize_ << std::endl;
        // 通知等待的任务可以开始工作了
        tasksNotEmptyCV_.notify_all();
    
        if (poolMode_ == PoolMode::MODE_CACHED &&
            taskSize_ > idleThreadSize_ &&
            curThreadSize_ < threadSizeThreshold_
            ) {
                auto threadPtr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
                int newThreadId = threadPtr->getId();
                threads_.emplace(newThreadId, std::move(threadPtr));
                threads_[newThreadId]->start();
                curThreadSize_++;
                idleThreadSize_++;
            }
        // 返回任务提交结果
        return ResultType;
    }

     // 启动线程池，设置初始线程数量
    void start(int initThreadSize = std::thread::hardware_concurrency())
    {
        isPoolRunning_ = true;
        // 记录初始线程数量
        initThreadSize_ = initThreadSize;
        curThreadSize_ = initThreadSize;
        // 创建线程
        for (int i = 0; i < initThreadSize; ++i) {
            auto threadPtr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            threads_.emplace(threadPtr->getId(), std::move(threadPtr));
        }
        // 启动线程
        for (int i = 0; i < initThreadSize; ++i) {
            threads_[i]->start();
            idleThreadSize_++;
        }
    }
    // 停止线程池
    void stop(){} 

    ThreadPool(const ThreadPool&) = delete;  // 删除拷贝构造函数
    ThreadPool& operator=(const ThreadPool&) = delete;  // 删除赋值操作符
   
private:

    void threadFunc(const int threadId)
    {
        auto lastTime = std::chrono::high_resolution_clock::now();

        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(tasksMutex_);
                std::cout << "tid:" << std::this_thread::get_id() << 
                    "try get task" << std::endl;
                    // 任务队列无任务时，等待任务
                while (tasksQue_.size() == 0) {
                    if (!isPoolRunning_) {
                        threads_.erase(threadId);
                        std::cout << "tid:" << std::this_thread::get_id() << 
                            "exit " << std::endl;
                        exitCodeCV_.notify_all();
                        return;
                    }
                    if (poolMode_ == PoolMode::MODE_CACHED) {
                        if (std::cv_status::timeout == tasksNotEmptyCV_.wait_for(lock, std::chrono::seconds(1))) {
                            auto now = std::chrono::high_resolution_clock::now();
                            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                            // 线程空闲时间超过阈值，并且当前线程数量超过初始线程数量，则回收当前线程
                            if (dur.count() >= THREAD_MAX_IDLE_TIME &&
                                curThreadSize_ > initThreadSize_) {
                                // 开始回收当前线程
                                // 把线程对象从线程池中移除
                                threads_.erase(threadId);
                                // 线程数量相关变量值修改
                                idleThreadSize_--;
                                curThreadSize_--;
                                std::cout << "tid:" << std::this_thread::get_id() << 
                                        "is exiting" << std::endl;
                                return;
                            }
                        }
                    } else {
                        tasksNotEmptyCV_.wait(lock);
                    }

                    /*if(!isPoolRunning_) {
                        // 线程池正在停止，回收线程资源
                        threads_.erase(threadId);
                        std::cout << "tid:" << std::this_thread::get_id() << 
                            "exit " << std::endl;
                        exitCodeCV_.notify_all();
                        return;
                    }*/
                } 

                idleThreadSize_--;
                std::cout << "tid:" << std::this_thread::get_id() << 
                    "get a task" << std::endl;
                task = tasksQue_.front();
                tasksQue_.pop();
                taskSize_--;
                if (tasksQue_.size() > 0)
                    tasksNotFullCV_.notify_all();
                tasksNotFullCV_.notify_all();
            }
            if (task != nullptr) {
                task();
                
            }
            lastTime = std::chrono::high_resolution_clock::now();
            idleThreadSize_++;
        }
    }
    bool CheckPoolRunning() const { return isPoolRunning_; }

    bool isPoolRunning_ ;
    PoolMode poolMode_;                 // 线程池模式
    size_t initThreadSize_;             // 初始线程数量
    std::atomic<int> idleThreadSize_;   // 空闲线程数量
    size_t threadSizeThreshold_;        // 线程数量阈值
    std::atomic<int> curThreadSize_;       // 线程数量

    // key=threadId, value=std::unique_ptr<Thread>
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;  // 线程池
    // Task任务
    using Task = std::function<void()>;
    std::queue<Task> tasksQue_;           // 任务队列（直接存放可调用对象）
    std::atomic<int> taskSize_;
    int taskSizeThreshold_;  // 任务数量阈值

    std::mutex tasksMutex_;         // 任务队列互斥锁   
    std::condition_variable tasksNotEmptyCV_;  // 任务队列不为空的条件变量
    std::condition_variable tasksNotFullCV_;   // 任务队列不为满的条件变量
    std::condition_variable exitCodeCV_;   // 线程退出码条件变量
};

#endif
