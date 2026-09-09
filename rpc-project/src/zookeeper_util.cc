#include "zookeeper_util.h"
#include "mprpc_application.h"


using namespace std;
using namespace mprpc;

void global_watcher(zhandle_t* zh, int type, int state, const char* path, void* context)
{
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE)
    {
        sem_t* sem = (sem_t*)context;
        sem_post(sem);
    }
}

ZkClient::ZkClient()
    : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

void ZkClient::Start()
{
    const string host = Config().LoadConfigValue("zookeeper_server_ip");
    const string port = Config().LoadConfigValue("zookeeper_server_port");
    string connStr = host + ":" + port;
    m_zhandle = zookeeper_init(connStr.c_str(), global_watcher, 3000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr)
    {
        cout << "zookeeper_init failed!" << endl;
    }
    cout << "zookeeper_init success!" << endl;

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    sem_wait(&sem);
    cout << "zookeeper connected!" << endl;
}

void ZkClient::Create(const std::string& path, const std::string& data, int state)
{
    char pathBuffer[128] = {0};
    const int buffLen = sizeof(pathBuffer);
    int flag = zoo_exists(m_zhandle, path.c_str(), 0, nullptr);
    if (flag == ZNONODE)
    {
        flag = zoo_create(m_zhandle, path.c_str(), data.c_str(), data.size(),
             &ZOO_OPEN_ACL_UNSAFE, state, pathBuffer, buffLen);
        if (flag != ZOK)
        {
            cout << "zoo_create failed! ret:" << flag << endl;
            cout << "flag:" << flag << ", path:" << path << endl;
            exit(EXIT_FAILURE);
        }
        cout << "zoo_create success!" << endl;
    }
}

const std::string ZkClient::GetData(const std::string& path)
{
    char dataBuffer[128] = {0};
    int buffLen = sizeof(dataBuffer);
    int ret = zoo_get(m_zhandle, path.c_str(), 0, dataBuffer, &buffLen, nullptr);
    if (ret != ZOK)
    {
        cout << "zoo_get failed! ret:" << ret << endl;
        return "";
    }
    cout << "zoo_get success!" << endl;
    cout << "data:" << dataBuffer << endl;
    return dataBuffer;
}
