/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-27 09:32:16
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-28 10:23:12
 */
#include "leileilei.h"

leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void test_sleep()
{
    leileilei::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        LEI_LOG_DEBUG(g_logger) << "sleep 2";
    });
    iom.schedule([](){
        sleep(10);
        LEI_LOG_DEBUG(g_logger) << "sleep 3";
    });
    LEI_LOG_DEBUG(g_logger) << "test sleep";
}

int main()
{
    test_sleep();
    return 0;
}

