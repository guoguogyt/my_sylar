/*** 
 * @Author: leileilei
 * @Date: 2022-06-22 16:23:43
 * @LastEditTime: 2022-06-23 21:00:29
 * @LastEditors: Please set LastEditors
 * @Description: 日志模块的具体实现
 * @FilePath: \my_sylar\leileilei\log.cpp
 * @
 */


#include "log.h"

namespace leileilei{

//LogLevel
std::string LogLevel::levelToString(LogLevel::level level)
{
    switch (level)
    {
#define XX(name)    \
    case LogLevel::name:  \
        return #name;   \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::level LogLevel::stringToLevel(const std::string& str)
{
#define XX(level, v)    \
    if(str == #v)   \
    {   \
        return LogLevel::level; \
    }

    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);

    return LogLevel::UNKNOW;
#undef XX
}

// LogEvent
LogEvent::LogEvent(const char* file_name, int32_t line, uint32_t elapse, 
            uint32_t thread_id, uint32_t fiber_id, uint64_t time, 
            std::string thread_name, LogLevel::level level)
{
    file_name_ = file_name;
    line_ = line;
    elapse_ = elapse;
    thread_id_ = thread_id;
    fiber_id_ = fiber_id;
    time_ = time;
    thread_name_ = thread_name;
    level_ = level;
}

//LogFormatter
LogFormatter::LogFormatter(std::string format)
{
    format_ = format;
}

LogFormatter::init()
{
    if(format_.empty()) format_ = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    //存储解析出来的格式    tuple中的元素表示(内容 格式  是否需要格式化)
    std::vector<std::tuple<std::string, std::string, int> > vec;
    

}


}
