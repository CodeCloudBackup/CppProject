#ifndef LOGGER_H
#define LOGGER_H

#include "lock_queue.h"
#include <atomic>
#include <string>
#include <sstream>
#include <thread>
#include <cstdio>
#include <vector>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& GetInstance();

    // 设置全局日志级别
    void SetLogLevel(LogLevel level);

    // 投递一条日志到异步队列
    void Log(const std::string& message, LogLevel level);

    // 立即刷新已投递日志到磁盘（阻塞等待后台线程处理完当前队列）
    void Flush();

    // 停止后台线程并关闭文件；析构时会自动调用
    void Stop();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void WorkerLoop();
    void OpenLogFile();
    void RotateLogFile();
    void WriteLog(const std::string& line);
    void ShiftAndArchive(const std::string& basePath);
    void CompressFile(const std::string& path);

    std::string GetCurrentDate() const;
    std::string GetTimestamp() const;
    std::string FormatLogLine(LogLevel level, const std::string& message) const;

    std::atomic<LogLevel> logLevel_;
    LockQueue<std::string> logQueue_;
    std::thread workerThread_;
    std::atomic<bool> running_;

    FILE* currentFile_;
    std::string currentFileName_;
    std::string currentDate_;
    size_t currentFileSize_;

    // 单个日志文件大小上限：10 MB
    static const size_t kMaxFileSize = 10 * 1024 * 1024;
    // 保留归档数量上限（.1.gz ~ .5.gz）
    static const int kMaxArchiveCount = 5;
};

// 内部辅助：从完整路径中提取文件名
inline const char* _extract_filename(const char* filepath) {
    const char* name = filepath;
    for (const char* p = filepath; *p; ++p) {
        if (*p == '/' || *p == '\\') name = p + 1;
    }
    return name;
}

// 内部辅助：将单个参数转为 string（兼容 const char* 和 std::string）
inline std::string _to_str(const char* s) { return s ? s : "(null)"; }
inline std::string _to_str(const std::string& s) { return s; }
template<typename T>
inline std::string _to_str(const T& v) { return std::to_string(v); }

// 内部辅助：无额外参数时直接返回消息
inline std::string _format_log(const char* msg) { return msg; }
inline std::string _format_log(const std::string& msg) { return msg; }

// 内部辅助：格式化日志（%s 兼容 std::string 和 const char*，%d 等兼容数值类型）
template<typename... Args>
inline std::string _format_log(const char* fmt, const Args&... args) {
    const char* p = fmt;
    size_t idx = 0;
    std::vector<std::string> parts = { _to_str(args)... };
    std::string result;
    while (*p) {
        if (*p == '%' && *(p + 1) != '%' && idx < parts.size()) {
            result += parts[idx++];
            while (*p && *p != '%') ++p;
            if (*p == '%') ++p;
            while (*p && *p != 's' && *p != 'd' && *p != 'f' && *p != 'l' && *p != 'u' && *p != 'x' && *p != 'X' && *p != 'o' && *p != 'e' && *p != 'E' && *p != 'g' && *p != 'G' && *p != 'i' && *p != 'c') ++p;
            if (*p) ++p;
        } else {
            result += *p++;
        }
    }
    return result;
}

#define LOG_DEBUG(logmsg, ...) \
    do { \
        std::string _msg = std::string("[") + _extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + "] " + _format_log(logmsg, ##__VA_ARGS__); \
        Logger::GetInstance().Log(_msg, LogLevel::DEBUG); \
    } while(0)

#define LOG_INFO(logmsg, ...) \
    do { \
        std::string _msg = std::string("[") + _extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + "] " + _format_log(logmsg, ##__VA_ARGS__); \
        Logger::GetInstance().Log(_msg, LogLevel::INFO); \
    } while(0)

#define LOG_WARNING(logmsg, ...) \
    do { \
        std::string _msg = std::string("[") + _extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + "] " + _format_log(logmsg, ##__VA_ARGS__); \
        Logger::GetInstance().Log(_msg, LogLevel::WARNING); \
    } while(0)

#define LOG_ERROR(logmsg, ...) \
    do { \
        std::string _msg = std::string("[") + _extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + "] " + _format_log(logmsg, ##__VA_ARGS__); \
        Logger::GetInstance().Log(_msg, LogLevel::ERROR); \
    } while(0)

#define LOG_FATAL(logmsg, ...) \
    do { \
        std::string _msg = std::string("[") + _extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + "] " + _format_log(logmsg, ##__VA_ARGS__); \
        Logger::GetInstance().Log(_msg, LogLevel::FATAL); \
        Logger::GetInstance().Flush(); \
        std::abort(); \
    } while(0)

#endif
