#include<stdio.h>
#include<iostream>
#include"log.h"

int main(int argc, char* argv[])
{
    // leileilei::LogLevel l;
    // std::cout<<l.stringToLevel("error")<<std::endl;
    // std::cout<<l.levelToString(leileilei::LogLevel::DEBUG)<<std::endl;
    // leileilei::LogFormatter::ptr format(new leileilei::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));

    leileilei::LogManager lm;
    LEI_LOG_DEBUG(lm.getRootLogger())<<"test log";
    return 0;
}

