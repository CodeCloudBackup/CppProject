#include "logger.h"
#include <time.h>
#include <iostream>
#include <chrono>
#include <cstdio>
#include <cstdlib>

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : logLevel_(LogLevel::DEBUG),
      currentFile_(nullptr),
      currentFileSize_(0),
      running_(true) {
    currentDate_ = GetCurrentDate();
    OpenLogFile();
    workerThread_ = std::thread(&Logger::WorkerLoop, this);
}

Logger::~Logger() {
    Stop();
}

void Logger::SetLogLevel(LogLevel level) {
    logLevel_.store(level);
}

void Logger::Log(const std::string& message, LogLevel level) {
    if (level < logLevel_.load(std::memory_order_relaxed)) {
        return;
    }
    logQueue_.Push(FormatLogLine(level, message));
}

void Logger::Flush() {
    // 等待后台线程把队列中已有的消息全部落盘
    while (!logQueue_.Empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // 再让出一次时间片，确保 worker 完成最后一次 WriteLog
    std::this_thread::yield();
    if (currentFile_) {
        fflush(currentFile_);
    }
}

void Logger::Stop() {
    if (running_.exchange(false)) {
        logQueue_.Stop();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        if (currentFile_) {
            fflush(currentFile_);
            fclose(currentFile_);
            currentFile_ = nullptr;
        }
    }
}

void Logger::WorkerLoop() {
    while (running_.load(std::memory_order_relaxed)) {
        std::string message;
        if (!logQueue_.Pop(message)) {
            break;
        }
        WriteLog(message);
    }

    // 处理退出前残留的日志，避免丢失
    std::string message;
    while (logQueue_.TryPop(message)) {
        WriteLog(message);
    }
}

void Logger::OpenLogFile() {
    currentFileName_ = "mprpc_" + currentDate_ + ".log";
    currentFile_ = fopen(currentFileName_.c_str(), "a+");
    if (!currentFile_) {
        std::cerr << "Failed to open log file: " << currentFileName_ << std::endl;
        currentFileSize_ = 0;
        return;
    }
    if (fseek(currentFile_, 0, SEEK_END) != 0) {
        std::cerr << "Failed to seek log file: " << currentFileName_ << std::endl;
        fclose(currentFile_);
        currentFile_ = nullptr;
        currentFileSize_ = 0;
        return;
    }
    long size = ftell(currentFile_);
    currentFileSize_ = (size > 0 ? static_cast<size_t>(size) : 0);
}

void Logger::RotateLogFile() {
    if (currentFile_) {
        fflush(currentFile_);
        fclose(currentFile_);
        currentFile_ = nullptr;
    }

    if (!currentFileName_.empty()) {
        ShiftAndArchive(currentFileName_);
    }

    OpenLogFile();
}

void Logger::WriteLog(const std::string& line) {
    std::string date = GetCurrentDate();
    if (date != currentDate_) {
        currentDate_ = date;
        RotateLogFile();
    }

    if (currentFile_ && currentFileSize_ + line.size() > kMaxFileSize) {
        RotateLogFile();
    }

    if (!currentFile_) {
        OpenLogFile();
    }

    if (currentFile_) {
        if (fputs(line.c_str(), currentFile_) == EOF) {
            std::cerr << "Failed to write log: " << currentFileName_ << std::endl;
            return;
        }
        // 每次写入立即 flush，保证进程崩溃前最后一刻日志不丢失
        fflush(currentFile_);
        currentFileSize_ += line.size();
    }
}

void Logger::ShiftAndArchive(const std::string& basePath) {
    // 删除最老的压缩归档
    std::string oldest = basePath + "." + std::to_string(kMaxArchiveCount) + ".gz";
    remove(oldest.c_str());

    // 依次后移已有压缩归档
    for (int i = kMaxArchiveCount - 1; i >= 1; --i) {
        std::string oldPath = basePath + "." + std::to_string(i) + ".gz";
        std::string newPath = basePath + "." + std::to_string(i + 1) + ".gz";
        rename(oldPath.c_str(), newPath.c_str());
    }

    // 当前日志 -> .1
    std::string archivePath = basePath + ".1";
    if (rename(basePath.c_str(), archivePath.c_str()) != 0) {
        std::cerr << "Failed to rename log file: " << basePath << std::endl;
        return;
    }

    // 后台压缩 .1 -> .1.gz
    CompressFile(archivePath);
}

void Logger::CompressFile(const std::string& path) {
    std::thread t([path]() {
        std::string cmd = "gzip -f \"" + path + "\"";
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            // gzip 不可用或失败，保留未压缩的 .1 文件
        }
    });
    t.detach();
}

std::string Logger::GetCurrentDate() const {
    time_t now = time(nullptr);
    struct tm tmInfo;
#ifdef _WIN32
    localtime_s(&tmInfo, &now);
#else
    localtime_r(&now, &tmInfo);
#endif
    char buf[32];
    snprintf(buf, sizeof(buf), "%d-%02d-%02d",
             tmInfo.tm_year + 1900, tmInfo.tm_mon + 1, tmInfo.tm_mday);
    return std::string(buf);
}

std::string Logger::GetTimestamp() const {
    time_t now = time(nullptr);
    struct tm tmInfo;
#ifdef _WIN32
    localtime_s(&tmInfo, &now);
#else
    localtime_r(&now, &tmInfo);
#endif
    char buf[32];
    snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d",
             tmInfo.tm_year + 1900, tmInfo.tm_mon + 1, tmInfo.tm_mday,
             tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec);
    return std::string(buf);
}

std::string Logger::FormatLogLine(LogLevel level, const std::string& message) const {
    const char* levelStr = nullptr;
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "[DEBUG]";
            break;
        case LogLevel::INFO:
            levelStr = "[INFO]";
            break;
        case LogLevel::WARNING:
            levelStr = "[WARNING]";
            break;
        case LogLevel::ERROR:
            levelStr = "[ERROR]";
            break;
        case LogLevel::FATAL:
            levelStr = "[FATAL]";
            break;
        default:
            levelStr = "[UNKNOWN]";
            break;
    }
    return GetTimestamp() + " " + levelStr + " " + message + "\n";
}
