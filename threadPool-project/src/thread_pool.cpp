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

    // 通知等待的任务可以开始工作了
    tasksNotEmptyCV_.notify_all();
    // 返回任务提交结果
    return ResultType(task);
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
            std::cout << "tid:" << std::this_thread::get_id() << 
                "try get task" << std::endl;
            tasksNotEmptyCV_.wait(lock, [&]()->bool { return tasksQue_.size() > 0; });
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
            task->run();
            
        }
    }
}

void ThreadPool::stop()
{
}


void Thread::start() 
{   
    std::thread t(func_);
    t.detach();
}

