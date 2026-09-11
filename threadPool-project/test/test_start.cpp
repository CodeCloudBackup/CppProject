#include <iostream>
#include <thread>
#include <chrono>

#include "thread_pool.h"

class MyTask : public Task {
public:
    MyTask() = default;
    ~MyTask() = default;
    AnyType run() override {
        std::cout << "MyTask is running" << std::endl;
        std::cout << "tid: " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return ;
    }
};

class MyTask1 : public Task {
public:
    MyTask1(const int begin, const int end) : begin_(begin), end_(end) {}
    ~MyTask1() = default;
    AnyType run() override {
        int sum = 0;
        for (int i = begin_; i < end_; ++i) {
            sum += i;
        }
        std::cout << "tid: " << std::this_thread::get_id() << std::endl;
        std::cout << "begin: " <<  begin_ << " " << "end: " << end_ << std::endl;
        std::cout << "sum: " << sum << std::endl;
        return sum;
    }
private:
    int begin_;
    int end_;
};

int main()
{
    ThreadPool pool;
    pool.start(4);
    std::cout << "pool start" << std::endl;
    for (int i = 0; i < 4; ++i) {
        pool.submitTask(std::make_shared<MyTask>());
    }
    pool.submitTask(std::make_shared<MyTask1>(0, 1000));
    for (int i = 0; i < 4; ++i) {
        pool.submitTask(std::make_shared<MyTask1>(i * 1000 + 1, (i + 1) * 1000));
    }
    getchar();
}