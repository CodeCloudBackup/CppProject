#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include <chrono>

#include "thread_pool.h"
using namespace std;

int sum1(int a, int b) ;
int sum2(int a, int b, int c) ;

/*
    submitTask 可变参模板
    C++库 thread packaged_task(function 函数对象) async
*/
int main() {
    ThreadPool pool;
   // pool.setMode(PoolMode::MODE_CACHED);
    pool.start(2);

    future<int> result1 = pool.submitTask(sum1, 1, 2);
    future<int> result2 = pool.submitTask(sum1, 1, 2);
    future<int> result3 = pool.submitTask(sum2, 1, 2, 3);
    future<int> result4 = pool.submitTask([](int a, int b)->int { return a * b; }, 1, 2);
    future<int> result5 = pool.submitTask(sum1, 1, 2);
    future<int> result6 = pool.submitTask(sum2, 1, 2, 3);

    cout << result1.get() << endl;
    cout << result2.get() << endl;
    cout << result3.get() << endl;
    cout << result4.get() << endl;
    cout << result5.get() << endl;
    cout << result6.get() << endl;
}

int sum1(int a, int b) {
    this_thread::sleep_for(chrono::seconds(3));
    return a + b;
}

int sum2(int a, int b, int c) {
    this_thread::sleep_for(chrono::seconds(3));
    return a + b + c;
}
