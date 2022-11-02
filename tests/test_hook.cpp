/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-27 09:32:16
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-02 10:44:16
 */
#include "leileilei.h"

leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void test_sleep()
{
    leileilei::IOManager iom(1);
    /**
     * @brief 
     * 这么使用，不会先sleep2s，在sleep10s
        会按照时间最长的sleep，既总休息10s
        因为可以当做schedule函数同时进入fiber list
        fiber list之间的执行也是连续的
     */
    iom.schedule([](){
        sleep(2);
        LEI_LOG_DEBUG(g_logger) << "sleep 2";
    });
    iom.schedule([](){
        sleep(10);
        LEI_LOG_DEBUG(g_logger) << "sleep 10";
    });
    LEI_LOG_DEBUG(g_logger) << "test sleep";
}

int main()
{
    test_sleep();
    return 0;
}

