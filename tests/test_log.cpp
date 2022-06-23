#include<stdio.h>
#include<iostream>
#include"log.h"

int main(int argc, char* argv[])
{
    leileilei::LogLevel l;
    std::cout<<l.stringToLevel("error")<<std::endl;
    std::cout<<l.levelToString(leileilei::LogLevel::DEBUG)<<std::endl;

    return 0;
}

