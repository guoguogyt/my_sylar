/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-27 11:23:18
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-27 11:27:35
 */
#include "leileilei.h"

void test_fun()
{
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << "doing task";
}

void test1()
{
    leileilei::IOManager io;
    io.schedule(&test_fun);
}

int main(int argc, char* argv[])
{
    test1();
    return 0;
}