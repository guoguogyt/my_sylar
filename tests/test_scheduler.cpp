/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-20 15:35:40
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-21 10:47:24
 */
#include "leileilei.h"

static leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void test_fiber()
{
    static int s_count = 5;
    LEI_LOG_DEBUG(g_logger) << "exe test_fiber function,and count=" <<s_count;
    sleep(1);
    if(--s_count >= 0)
    {
        leileilei::Scheduler::GetThis()->schedule(&test_fiber, leileilei::GetThreadId());
    }
}

int main(int argc, char* argv[])
{
    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/log.yml");
    leileilei::ConfigManager::LoadConfigFromYaml(root);
    LEI_LOG_DEBUG(g_logger) << "main start";
    sleep(2);
    leileilei::Scheduler sc(3, true, "test_scheduler");
    sleep(2);
    sc.start();
    sleep(2);
    LEI_LOG_DEBUG(g_logger) << "main other things";
    sc.schedule(&test_fiber);
    sleep(2);
    sc.stop();
    LEI_LOG_DEBUG(g_logger) << "main over";
}