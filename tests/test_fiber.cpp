/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-13 11:17:02
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-13 16:05:33
 */
#include "leileilei.h"

leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void run_in_fiber()
{
    LEI_LOG_DEBUG(g_logger) << "run in fiber begin";
    leileilei::Fiber::YieldToHold();
    LEI_LOG_DEBUG(g_logger) << "come sub fiber";
    leileilei::Fiber::YieldToHold();
}

void test_fiber()
{
    LEI_LOG_DEBUG(g_logger) << "test fiber begin";
    leileilei::Fiber::GetThis();
    LEI_LOG_DEBUG(g_logger) << "main fiber begin";
    leileilei::Fiber::ptr fiber_(new leileilei::Fiber(run_in_fiber));
    fiber_->swapIn();
    LEI_LOG_DEBUG(g_logger) << "come main fiber";
    fiber_->swapIn();
    LEI_LOG_DEBUG(g_logger) << "sub fiber end, come main fiber";
}

int main(int argc, char* argv[])
{
    test_fiber();
    return 0;
}