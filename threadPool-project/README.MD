# 线程池-学习项目

## 项目介绍
```text
作为五大池之一（内存池、连接池、线程池、进程池、协程池），线程池应用非常广泛，提高业务处理能力必备，各个开源线程池实现，基本原理基本相同。
本项目使用 C++17标准
```

### VScode编译命令
    清理cmake缓存
    del CMakeCache.txt
    cmd /c "rmdir /s /q CMakeFiles" 
    编译
    cmake .. -G "MinGW Makefiles"
    mingw32-make
    $env:PATH = "e:\docker\rpc-system\src\CppProject\threadPool-project\lib;" + $env:PATH