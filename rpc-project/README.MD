# RPC通信框架-学习项目

## 项目介绍
```text
简单的rpc框架学习项目，使用protobuf、muduo等组件
```
### RPC通信原理

```mermaid
graph LR
    %% 定义样式
    classDef user fill:#ffcccc,stroke:#333,stroke-width:1px;
    classDef server fill:#ffcccc,stroke:#333,stroke-width:1px;
    classDef stub fill:#ffffcc,stroke:#333,stroke-width:1px;
    classDef runtime fill:#ccffcc,stroke:#333,stroke-width:1px;
	classDef subgraphStyle fill:#ffffff,stroke:#333,stroke-width:2px,color:#333;
	
    subgraph Caller ["Caller (客户端)"]
        direction TB
        C_User[local call<br/><br/><br/><br/>local return]:::user
        C_Stub[pack argument<br/><br/><br/><br/>unpack result]:::stub
        C_Runtime[transmit<br/>↓<br/>wait<br/>↓<br/>receive]:::runtime
       
        C_User --> C_Stub
        C_Stub --> C_Runtime
        C_Runtime --> C_Stub
        C_Stub --> C_User
    end

    subgraph Callee ["Callee (服务端)"]
        direction TB
        S_Runtime[receive<br/><br/><br/><br/>transmit]:::runtime
        S_Stub[unpack argument<br/><br/><br/><br/>pack result]:::stub
        S_Server[call<br/>↓<br/>work<br/>↓<br/>return]:::server
       	
        S_Runtime --> S_Stub
        S_Stub --> S_Server
        S_Server --> S_Stub
        S_Stub --> S_Runtime
        
    end

    %% 跨网络通信
    C_Runtime ==>|Call packet| S_Runtime
    S_Runtime ==>|Result packet| C_Runtime
    %% 用 class 指令绑定 subgraph 样式
    class Caller subgraphStyle
    class Callee subgraphStyle
```

## 环境依赖

```
ubuntu 22.04
boost 1.174.0
muduo 2.0.2
zookeeper 3.4.13
```

## 目录结构

```
├── config  // 配置文件目录
├── example // 调用远程函数客户端和提供远程函数服务端函数示例
├── src     // rpc框架代码
│  ├── include    // rpc框架头文件目录
├── autobuild.sh  // 编译脚本
└── README.MD     // 代码说明
```

