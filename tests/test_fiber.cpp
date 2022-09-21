/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-13 11:17:02
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-21 09:57:43
 */

#include <vector>
#include <string>
#include "leileilei.h"

leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void run_in_fiber()
{
    printf("sub fiber_id = %d", leileilei::Fiber::GetFiberId());
    LEI_LOG_DEBUG(g_logger) << "run in fiber begin";
    leileilei::Fiber::YieldToHoldMainFiber();
    LEI_LOG_DEBUG(g_logger) << "come sub fiber";
    leileilei::Fiber::YieldToHoldMainFiber();
}

void test_fiber()
{
    LEI_LOG_DEBUG(g_logger) << "test fiber begin";
    {
        printf("main fiber_id = %d", leileilei::Fiber::GetFiberId());
        leileilei::Fiber::GetThis();
        LEI_LOG_DEBUG(g_logger) << "main fiber begin";
        leileilei::Fiber::ptr fiber_(new leileilei::Fiber(run_in_fiber, 0, true));
        fiber_->call();
        LEI_LOG_DEBUG(g_logger) << "come main fiber";
        fiber_->call();
        LEI_LOG_DEBUG(g_logger) << "sub fiber is swapOut, come main fiber";
        fiber_->call();
        LEI_LOG_DEBUG(g_logger) << "sub fiber callback exe end";
    }
    LEI_LOG_DEBUG(g_logger) << "sub fiber should destory, and then main fiber will be destory";
}

int main(int argc, char* argv[])
{
    // test_fiber();
    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/log.yml");
    leileilei::ConfigManager::LoadConfigFromYaml(root);
    std::vector<leileilei::Thread::ptr> vec;

    for(int i=0; i<1; i++)
    {
        vec.push_back(leileilei::Thread::ptr(new leileilei::Thread(&test_fiber, "name_" + std::to_string(i))));
    }

    for(int i=0; i<vec.size(); i++)
    {
        vec[i]->join();
    }

    return 0;
}