#include "thread_pool.h"

#include <chrono>

const int TASK_SIZE_THRESHOLD = 1024;
const int THREAD_MAX_THRESHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 60;

 ThreadPool::ThreadPool() 
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

ThreadPool::~ThreadPool()
{
    isPoolRunning_ = false;
    tasksNotEmptyCV_.notify_all();
    std::unique_lock<std::mutex> lock(tasksMutex_);
    exitCodeCV_.wait(lock, [&]()->bool { return tasksQue_.size() == 0; });
}

void ThreadPool::setMode(PoolMode mode)
{
    if (CheckPoolRunning()) {
        std::cerr << "Pool is running, cannot set mode" << std::endl;
        return;
    }
    poolMode_ = mode;
}

void ThreadPool::setTaskSizeThreshold(const int threshold) 
{
    taskSizeThreshold_ = threshold;
}

void ThreadPool::setThreadSizeThreshold(const int threshold) 
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

// 给线程池提交任务
ResultType ThreadPool::submitTask(std::shared_ptr<Task> task)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(tasksMutex_);
    // 等待任务队列有空闲位置
    // 用户指定等待时间，最长时间不能阻塞1s，否者判断任务提交失败
    if(!tasksNotFullCV_.wait_for(lock, std::chrono::seconds(1), 
        [&]()->bool { return tasksQue_.size() < (size_t)taskSizeThreshold_; }))
    {
        std::cerr << "Task queue is full, submit task timeout" << std::endl;
        return ResultType(task, false);
    }
    // 将任务放入任务队列
    tasksQue_.emplace(task);
    taskSize_++;
    std::cout << "Task submitted, task size: " << taskSize_ << std::endl;
    // 通知等待的任务可以开始工作了
    tasksNotEmptyCV_.notify_all();
    
    if (poolMode_ == PoolMode::MODE_CACHED &&
        taskSize_ > idleThreadSize_ &&
        curThreadSize_ < threadSizeThreshold_
        ) {
            auto threadPtr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            threads_.emplace(threadPtr->getId(), std::move(threadPtr));
            threads_[threadPtr->getId()]->start();
            curThreadSize_++;
            idleThreadSize_++;
        }
    // 返回任务提交结果
    return ResultType(task);
}

void ThreadPool::start(int initThreadSize) 
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

void ThreadPool::threadFunc(const int threadId) 
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (isPoolRunning_) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(tasksMutex_);
            std::cout << "tid:" << std::this_thread::get_id() << 
                "try get task" << std::endl;
                // 任务队列无任务时，等待任务
            while (tasksQue_.size() == 0) {
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

            if (!isPoolRunning_) {
                break;
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
            task->execute();
            
        }
        lastTime = std::chrono::high_resolution_clock::now();
        idleThreadSize_++;
    }
    threads_.erase(threadId);
    std::cout << "tid:" << std::this_thread::get_id() << 
        "exit " << std::endl;
    exitCodeCV_.notify_all();
}

void ThreadPool::stop()
{
}


int Thread::generateId_ = 0;

void Thread::start() 
{   
    std::thread t(func_, threadId_);
    t.detach();
}

