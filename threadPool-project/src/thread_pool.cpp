#include "thread_pool.h"

 const int TASK_SIZE_THRESHOLD = 4;

 ThreadPool::ThreadPool() 
 :initThreadSize_(0), 
 taskSize_(0),
 taskSizeThreshold_(TASK_SIZE_THRESHOLD), 
 poolMode_(PoolMode::MODE_FIXED) 
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::setMode(PoolMode mode)
{
    poolMode_ = mode;
}

void ThreadPool::setTaskSizeThreshold(int threshold) 
{
    taskSizeThreshold_ = threshold;
}

void ThreadPool::submitTask(std::shared_ptr<Task> task)
{
    std::unique_lock<std::mutex> lock(tasksMutex_);
    if (tasksQue_.size() >= taskSizeThreshold_) {
        tasksNotFullCV_.wait(lock);
    }
    // wait wait_for wait_until
    if(!tasksNotFullCV_.wait_for(lock, std::chrono::seconds(1), 
        [&]()->bool { return tasksQue_.size() < taskSizeThreshold_; })) {
        
        std::cerr << "Task queue is full, submit task timeout" << std::endl;
    }
    tasksQue_.push(task);
    taskSize_++;

    tasksNotEmptyCV_.notify_all();
}

void ThreadPool::start(int initThreadSize) 
{
    // 记录初始线程数量
    initThreadSize_ = initThreadSize;
    // 创建线程
    for (int i = 0; i < initThreadSize; ++i) {
        auto threadPtr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(threadPtr));
    }
    // 启动线程
    for (int i = 0; i < initThreadSize; ++i) {
        threads_[i]->start();
    }
}

void ThreadPool::threadFunc() 
{
    for (;;) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(tasksMutex_);
            tasksNotEmptyCV_.wait(lock, [&]()->bool { return tasksQue_.size() > 0; });
            task = tasksQue_.front();
            tasksQue_.pop();
            taskSize_--;
            if (tasksQue_.size() > 0)
                tasksNotFullCV_.notify_all();
        }
        if (task != nullptr) {
            task->run();

        }
    }
}

void ThreadPool::stop()
{
}

// 线程方法实现
Thread::~Thread()
{
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void Thread::start() 
{   
    thread_ = std::make_unique<std::thread>(func_);
}

void Thread::join()
{
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

std::thread::id Thread::getThreadId() const
{
    if (thread_) {
        return thread_->get_id();
    }
    return std::thread::id();
}


