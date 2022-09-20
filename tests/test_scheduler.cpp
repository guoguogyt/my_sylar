/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-20 15:35:40
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-20 16:26:14
 */
#include "leileilei.h"

static leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void test_fiber()
{
    LEI_LOG_DEBUG(g_logger) << "exe test_fiber function";
}

int main(int argc, char* argv[])
{
    LEI_LOG_DEBUG(g_logger) << "main start";
    leileilei::Scheduler sc(1, true, "test_scheduler");
    sc.start();
    LEI_LOG_DEBUG(g_logger) << "main other things";
    sc.schedule(&test_fiber)
    sc.stop();
    LEI_LOG_DEBUG(g_logger) << "main over";
}