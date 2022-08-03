#include<stdio.h>
#include<iostream>
#include"log.h"

int main(int argc, char* argv[])
{
    // leileilei::LogLevel l;
    // std::cout<<l.stringToLevel("error")<<std::endl;
    // std::cout<<l.levelToString(leileilei::LogLevel::DEBUG)<<std::endl;
    // leileilei::LogFormatter::ptr format(new leileilei::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));

    // leileilei::LogManager lm;
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR())<<"test log";
    LEI_FMT_LOG_INFO(LEI_LOG_GETROOTOR(), "%s", "format log");

    Logger::ptr root = LEI_LOG_GETROOTOR();
    LogAppender::ptr appender(new FileLogAppender("test.log"));
    appender->resetFormat("%d%T%m%n");
    root->addAppender(appender);

    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR())<<"test log";
    LEI_FMT_LOG_INFO(LEI_LOG_GETROOTOR(), "%s", "format log");
    return 0;
}

