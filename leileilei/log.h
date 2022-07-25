
/*** 
 * @Author: leileilei
 * @Date: 2022-06-22 16:23:37
 * @LastEditTime: 2022-06-27 21:21:08
 * @LastEditors: Please set LastEditors
 * @Description: 日志模块的头文件
 *  顶层 ： LogManager日志器管理者    以map的形式管理数个Logger，可以删除、添加、获取某个日志器，默认生成主日志器
 *          Logger日志器             以vector管理数个appender，可以添加删除appender，有写日志的方法
 *          Appender输出器           输出器控制输出的地方，控制某个日志级别是否可以输出，控制输出的格式(控制LogFormatter)
 *          LogFormatter格式器       以某种格式输出日志 可以修改格式
 *  底层：  LogEvent日志事件          事件中有日志的所有信息，但是经过LogFormatter之后信息并不会全部输出，会按照指定的格式进行输出 
 * @FilePath: \my_sylar\leileilei\log.h
 * @
 */

#pragma once


#include<memory>//智能指针的头文件
#include<string>
#include<sstream>
#include<fstream>
#include<vector>
#include<iostream>
#include<map>
#include<stdarg.h>
#include"singleton.h"
#include"config.h"

/*
    流式输出日志
*/
#define LEI_LOG_LEVEL(logger, level) \
    leileilei::LogEventWrap(logger, leileilei::LogEvent::ptr(new leileilei::LogEvent(__FILE__, \
                                __LINE__, 0, 1, 2, time(0), "threadName", level))).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define LEI_LOG_DEBUG(logger) LEI_LOG_LEVEL(logger, leileilei::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define LEI_LOG_INFO(logger) LEI_LOG_LEVEL(logger, leileilei::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define LEI_LOG_WARN(logger) LEI_LOG_LEVEL(logger, leileilei::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define LEI_LOG_ERROR(logger) LEI_LOG_LEVEL(logger, leileilei::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define LEI_LOG_FATAL(logger) LEI_LOG_LEVEL(logger, leileilei::LogLevel::FATAL)

/*
    格式化式输出日志
*/
#define LEI_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    leileilei::LogEventWrap(logger, leileilei::LogEvent::ptr(new leileilei::LogEvent(__FILE__, \
                                __LINE__, 0, 1, 2, time(0), "threadName", level))).getLogEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化式方式将日志级别debug的日志写入到logger
 */
#define LEI_FMt_LOG_DEBUG(logger, fmt, ...) LEI_LOG_FMT_LEVEL(logger, leileilei::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化式方式将日志级别info的日志写入到logger
 */
#define LEI_FMT_LOG_INFO(logger, fmt, ...) LEI_LOG_FMT_LEVEL(logger, leileilei::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化式方式将日志级别warn的日志写入到logger
 */
#define LEI_FMT_LOG_WARN(logger, fmt, ...) LEI_LOG_FMT_LEVEL(logger, leileilei::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化式方式将日志级别error的日志写入到logger
 */
#define LEI_FMT_LOG_ERROR(logger, fmt, ...) LEI_LOG_FMT_LEVEL(logger, leileilei::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化式方式将日志级别fatal的日志写入到logger
 */
#define LEI_FMT_LOG_FATAL(logger, fmt, ...) LEI_LOG_FMT_LEVEL(logger, leileilei::LogLevel::FATAL, fmt ,__VA_ARGS__)

/**
 * @brief 获取主日志器
 */
 #define LEI_LOG_GETROOTOR()    leileilei::SingLogMar::GetInstance()->getRootLogger()

/**
 * @brief 按照name获取日志器
 */
#define LEI_GET_LOGGER(name)    leileilei::SingLogMar::GetInstance()->getLogger(name)

namespace leileilei{

class Logger;

/*** 
 * @description: 日志级别
 */
class LogLevel
{
public:
    enum level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };
    //将level类型转为对应的字符串
    static std::string levelToString(LogLevel::level level);
    //将字符串转化为对应的level类型
    static LogLevel::level stringToLevel(const std::string& str);
};

/*** 
 * 日志事件
 * @description: 要尽量的全，可以在format的时候不用，但是这里要尽量的全
 * @return {*}
 */
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    /**
     * @brief 构造函数
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     *              时间    线程id  线程名称   协程id   [日志级别]  [日志名称]  文件名:行号 消息\n 
     */
    LogEvent(const char* file_name, int32_t line, uint32_t elapse, 
            uint32_t thread_id, uint32_t fiber_id, uint64_t time, 
            std::string thread_name, LogLevel::level level);
    //文件名称操作
    void setFilename(char* filename) {file_name_ = filename;}
    const char* getFilename() const { return file_name_;}
    //行号操作
    void setLine(int32_t line) {line_ = line;}
    int32_t getLine() const {return line_;}
    //截止毫秒数操作
    void setElapse(uint32_t elapse) {elapse_ = elapse;}
    uint32_t getElapse() const {return elapse_;}
    //线程id操作
    void setThreadID(uint32_t thread_id) { thread_id_ = thread_id;}
    uint32_t getThreadID() const { return thread_id_;}
    //协程id操作
    void setFiberID(uint32_t fiber_id) { fiber_id_ = fiber_id;}
    uint32_t getFiberID() const {return fiber_id_;}
    //时间戳操作
    void setTime(uint64_t time) { time_ = time;}
    uint64_t getTime() const {return time_;}
    //日志流内容操作
    void setSS(std::stringstream ss) {ss_ << ss;}
    std::stringstream& getSS() {return ss_;}
    //返回日志内容
    std::string getContent() { return ss_.str();}
    //日志级别操作
    void setLevel(LogLevel::level level) { level_ = level;}
    LogLevel::level getLevel() const {return level_;}
    //线程名称操作
    void setThreadID(std::string thread_name) {thread_name_ = thread_name;}
    std::string getThreadName() {return thread_name_;}

    //格式化方式写入日志
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

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

/*** 
 * @description:日志格式器 
 * 默认格式为%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
 */
class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    //构造函数 默认格式为%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    LogFormatter(std::string format = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
    //在构造时被调用，解析格式
    void init();
    //生成日志，将产生的日志事件按照所存储的格式解析，将解析后的结果以字符串的形式返回
    std::string doFormat(std::shared_ptr<Logger> logger, LogEvent::ptr event);
    //生成日志，将产生的日志事件按照所存储的格式解析，将解析后的结果以流的形式返回
    std::ostream& doFormat(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event);
    //重新设置解析模板
    bool resetFormat(std::string format);
    //得到模板
    std::string getFormat() const { return format_;}
public:
    //定义一个内部类用来保存解析出来的每一项格式
    class FormatItem
    {   
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        /*
            析构函数为虚函数
            class A{};
            class B:public A
            当：
                A* a = new B;
            当a被释放时，执行了a的析构函数。假如b中有new出的变量，则变量不会被清除，导致内存泄漏
        */
        virtual ~FormatItem() {}
        //纯虚函数需要子类实现
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) = 0;
    };
private:
    std::string format_;
    std::vector<FormatItem::ptr> items_;

};

/*** 
 * 日志输出地 默认日志级别是DEBUG  无默认的formatter，需要设置
 * @description:Appender中包含Formatter，不同的的Appender应该允许输出的日志格式不一样，并且不同的appender可以控制 日志级别 进行选择性输出日志
 */
class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    //设置默认的日志级别
    LogAppender(LogLevel::level level = LogLevel::level::DEBUG){}
    //生成日志，纯虚函数需要子类实现
    virtual void doLog(std::shared_ptr<Logger> logger, LogEvent::ptr event) = 0;
    //重新设置formatter
    void resetFormat(std::string format);
    void resetFormat(LogFormatter::ptr formart);
    //获取formatter
    LogFormatter::ptr getFormat() const { return format_ptr_;}
    //设置日志级别
    void setLevel(LogLevel::level level) {level_ = level;}
    //获取日志级别
    LogLevel::level getLevel() const { return level_;}
private:
    LogFormatter::ptr format_ptr_;
    LogLevel::level level_;//默认是debug
};

/*** 
 * @description: 输出到控制台
 */
class StdoutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    //重写父类的纯虚函数
    void doLog(std::shared_ptr<Logger> logger, LogEvent::ptr event) override;
};

/*** 
 * @description: 输出到文件
 * 还有待完成
 */
class FileLogAppender : public LogAppender
{
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& fliename);
    //重写父类的纯虚函数
    void doLog(std::shared_ptr<Logger> logger, LogEvent::ptr event) override;
    //重新打开
    bool reopen();
private:
    //文件名 含路径
    std::string filename_;
    //文件流
    std::ofstream filestream_;
    //上次打开的时间
    uint64_t last_time_;
};

/*** 
 * @description:日志器
 * 日志器中应该包含1个或者多个Appender 
 * 
 */
static uint64_t logger_default_name_ = 0;
class Logger : public std::enable_shared_from_this<Logger>
{
public:
    typedef std::shared_ptr<Logger> ptr;
    //构造函数中自动生成一个名称
    Logger();
    //写日志
    void doLog(LogEvent::ptr event);
    //设置日志器名称
    void setLoggerName(std::string name) { name_ = name;}
    //获取日志器名称
    std::string getLoggerName() { return name_;}
    //添加一个appner
    void addAppender(LogAppender::ptr appender);
    //删除一个appender
    void delAppender(LogAppender::ptr appender);
    //获取某个appender
    LogAppender::ptr getAppender(int index);
private:
    //日志器的名称
    std::string name_;
    //Appender集合
    std::vector<LogAppender::ptr> appenders_;
    
};

/*** 
 * @description: 日志器管理
 * 日志器管理单利模式，程序中应该只有一个实例
 * 日志的生成和获取都应该通过LogManager操作
 * 默认会生成一个名叫做root的主日志器
 * 主日志器的appender，默认2个 一个是流 一个是文件
 * 流appender的日志级别为debug，文件的日志级别为warn 
 */
class LogManager
{
public:
    //构造函数
    LogManager();
    //按名称获取logger，如果没有则创建
    Logger::ptr getLogger(std::string name);
    //获取主日志器
    Logger::ptr getRootLogger() {   return root_logger_;}
    //删除某个日志器
    void delLogger(std::string name);
    //添加某个日志器
    bool addLogger(Logger::ptr logger);
private:
    std::map<std::string, Logger::ptr> name_logger_;
    Logger::ptr root_logger_;
};

class LogEventWrap
{
public:
    //构造函数
    LogEventWrap(Logger::ptr logger, LogEvent::ptr e);
    //析构函数
    ~LogEventWrap();
    //获取日志事件
    LogEvent::ptr getLogEvent() { return event_;}
    //获取日志器
    Logger::ptr grtLogger() { return logger_;}
    //获取日志事件流
    std::stringstream& getSS() { return event_->getSS();}
private:
    LogEvent::ptr event_;
    Logger::ptr logger_;
};


typedef leileilei::Singleton<LogManager> SingLogMar;

}

