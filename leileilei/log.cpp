/*** 
 * @Author: leileilei
 * @Date: 2022-06-22 16:23:43
 * @LastEditTime: 2022-06-25 18:14:01
 * @LastEditors: Please set LastEditors
 * @Description: 日志模块的具体实现
 * @FilePath: \my_sylar\leileilei\log.cpp
 * @
 */


#include "log.h"
#include <functional>
#include <time.h>

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
    init();
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << LogLevel::levelToString(event->getLevel());
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << logger->getLoggerName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getThreadID();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getFiberID();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getFilename();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << std::endl;
    }
};


class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << "\t";
    }
// private:
//     std::string m_string;
};

void LogFormatter::init()
{
    if(format_.empty()) format_ = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    //存储解析出来的格式    tuple中的元素表示(内容 格式  是否需要格式化)
    std::vector<std::tuple<std::string, std::string, int> > vec;
    //表示纯文本内容，不需要格式化
    std::string purity_str;
    
    for(int i=0;i<format_.size();i++)
    {
        if(format_[i] != '%')//不需要格式化,直接将其放入purity_str
        {
            purity_str.append(1,format_[i]);
            continue;
        }

        if(i+1 < format_.size())//是否为%符号
        {
            if(format_[i+1] == '%')
            {
                purity_str.append(1,format_[i]);
                i++;
                continue;
            }
        }

        int n = i+1;
        int status = 0;
        std::string fmt_str;
        std::string str;
        while (n < format_.size())
        {
            if(status==0 && format_[n]=='%') break;//解析完成一段
            if(status==0 && format_[n]=='{')//进入格式
            {
                status = 1;
                n++;
                continue;
            }
            if(status==0 && isalpha(format_[n]))//如果%之后紧跟的是一个字符，那个保存这个字符
            {
                str.append(1, format_[n]);
                n++;
                if(n < format_.size() && format_[n]=='{') continue;//字符之后还有格式
                else break;//没有格式退出，因为规定格式%之后只有1个字符
            }
            if(status == 1)
            {
                if(format_[n] == '}') //格式解析结束
                {
                    status = 0;
                    n++;
                    break;
                }
                fmt_str.append(1, format_[n]);
                n++;
            }
        }
        if(status == 0)
        {
            if(!purity_str.empty())//非空放入
            {
                vec.push_back(std::make_tuple(purity_str, std::string(), 0));
                purity_str.clear();
            }
            vec.push_back(std::make_tuple(str, fmt_str, 1));//将解析出的格式放入
            i = n-1;
        }
        else if(status == 1)
        {
            vec.push_back(std::make_tuple("<输入格式错误[Input format error]>", fmt_str , 0));
        }
    }
    if(!purity_str.empty())//非空放入
    {
        vec.push_back(std::make_tuple(purity_str, std::string(), 0));
        purity_str.clear();
    }

    // --到此已经将格式拆分成一个一个单元放入vec中，接下来要为不同格式匹配不同的处理事件
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> >  format_function_ = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)) ; }}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };
    
    //放入之前保证items_是干净的
    std::vector<FormatItem::ptr>().swap(items_);
    //将解析到的格式放入items_中，这样使用时只需要便利items_中的format函数即可拼接成完整的日志格式
    for(int i=0;i<vec.size();i++)
    {
        if(std::get<2>(vec[i]) == 0)
        {
            items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(vec[i]))));
        }
        else
        {
            auto it = format_function_.find(std::get<0>(vec[i]));
            if(it == format_function_.end())
            {
                items_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(vec[i]) + ">>")));
            }
            items_.push_back(it->second(std::get<1>(vec[i])));
        }
    }

}

std::string LogFormatter::doFormat(std::shared_ptr<Logger> logger, LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto& it : items_)
    {
        it->format(ss, logger, event);
    }
    return ss.str();
}
//注意引用
std::ostream& LogFormatter::doFormat(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event)
{
    for(auto& it : items_)
    {
        it->format(os, logger, event);
    }
    return os;
}

bool LogFormatter::resetFormat(std::string format)
{
    format_ = format;
    init();
}

void LogAppender::resetFormat(std::string format)
{
    LogFormatter::ptr format_ptr(new LogFormatter(format));
    resetFormat(format_ptr);
}

void LogAppender::resetFormat(LogFormatter::ptr formart)
{
    if(formart)
    {
        format_ptr_ = formart;
    }
}

void StdoutLogAppender::doLog(Logger::ptr logger, LogEvent::ptr event)
{
    //是否有格式
    if(getFormat())
    {
        //是否达到了日志输出级别限制
        if(event->getLevel() >= getLevel())
            getFormat()->doFormat(std::cout, logger, event);
    }
    else 
        std::cout<<"日志模板器为空，无法生成日志！"<<std::endl;
}

FileLogAppender::FileLogAppender(const std::string& fliename)
{
    filename_ = fliename;
    reopen();
}

bool FileLogAppender::reopen()
{
    if(filestream_) {
        filestream_.close();
    }
    //这里将文件的相关操作封装到until类中，暂时先空
    return true;
}

void FileLogAppender::doLog(std::shared_ptr<Logger> logger, LogEvent::ptr event)
{
    //是否有格式
    if(getFormat())
    {
        //是否达到了日志输出级别限制
        if(event->getLevel() >= getLevel())
        {
            uint64_t now_time = event->getTime();
            if(now_time > last_time_ + 3)
            {
                reopen();
                last_time_ = now_time;
            }            
            getFormat()->doFormat(filestream_, logger, event);
        }
    }
    else 
        std::cout<<"日志模板器为空，无法生成日志！"<<std::endl;
}

Logger::Logger()
{
    logger_default_name_++;
    // name_ = std::string(default_name_);
}

void Logger::doLog(LogEvent::ptr event)
{
    auto self = shared_from_this();
    for(int i=0; i<appenders_.size(); i++)
    {
        appenders_[i]->doLog(self, event);
    }
}

void Logger::addAppender(LogAppender::ptr appender)
{
    
}

void Logger::delAppender(LogAppender::ptr appender)
{

}

LogAppender::ptr Logger::getAppender(int index)
{

}

}
