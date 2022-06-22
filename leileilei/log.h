/*** 
 * @Author: leileilei
 * @Date: 2022-06-22 16:23:37
 * @LastEditTime: 2022-06-22 22:22:00
 * @LastEditors: Please set LastEditors
 * @Description: 日志模块的头文件
 * @FilePath: \my_sylar\leileilei\log.h
 * @
 */

#pragma once
#include<memory>//智能指针的头文件
#include<string>
#include<sstream>

namespace leileilei{

class LogLevel
{
public:
    enum level{
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };
    //将level类型转为对应的字符串
    std::string levelToString(LogLevel::level level);
    //将字符串转化为对应的level类型
    LogLevel::level stringToLevel(std::string level);
};

class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(const char* file_name, int32_t line, uint32_t elapse, 
            uint32_t thread_id, uint32_t fiber_id, uint64_t time, 
            std::string thread_name, LogLevel::level level);
    //文件名称操作
    void setFilename();
    char* getFilename();
    //行号操作
    void setLine();
    int32_t getLine();
    //截止毫秒数操作
    void setElapse();
    uint32_t getElapse();
    //线程id操作
    void setThreadID();
    uint32_t getThreadID();
    //协程id操作
    void setFiberID();
    uint32_t getFiberID();
    //时间戳操作
    void setTime();
    uint64_t getTime();
    //日志流内容操作
    void setSS();
    std::stringstream getSS();
    //日志级别操作
    void setLevel();
    LogLevel::level getLevel();

private:
    //文件名称
    const char* file_name_;
    //行号
    int32_t line_;
    //程序启动到现在的毫秒数
    uint32_t elapse_;
    //线程id
    uint32_t thread_id_;
    //协程ID
    uint32_t fiber_id_;
    //时间戳
    uint64_t time_;
    //线程名称
    std::string thread_name_;
    //日志流内容
    std::stringstream ss_;
    //日志级别
    LogLevel::level level_;

};

class LogFormatter
{

};

class LogAppender
{

};

class Logger
{
    
};

class LogManager
{

};




}

