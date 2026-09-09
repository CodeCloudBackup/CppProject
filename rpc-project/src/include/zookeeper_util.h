#ifndef ZOOKEEPER_UTIL_H
#define ZOOKEEPER_UTIL_H

#include <iostream>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // 连接ZKServer
    void Start();
    // 创建Znode
    void Create(const std::string& path, const std::string& data, int state = 0);
    const std::string GetData(const std::string& path);
private:
    zhandle_t* m_zhandle;
};


#endif
