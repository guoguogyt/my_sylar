#include "leileilei.h"
#include <vector>
#include <unistd.h>

int count = 0;

leileilei::Logger::ptr logs = LEI_LOG_GETROOTOR();

void fun1()
{
    // LEI_LOG_INFO(logs) << "name: " << leileilei::Thread::GetThreadName()
    //                     << " this.name: " << leileilei::Thread::getThis()->getThreadName()
    //                     << " id: " << leileilei::GetThreadId()
    //                     << " this.id: " << leileilei::Thread::getThis()->getThreadId();

    for(int i=0;i<10000000;i++)
    {
        ++count;
    }
    LEI_LOG_INFO(logs) << "name: " << leileilei::Thread::GetThreadName() << " count = "<< count;
}

void fun2()
{
    while(true)
    {
        LEI_LOG_INFO(logs) << "--------------------------------";
    }
}

void fun3()
{
    while(true)
    {
        LEI_LOG_INFO(logs) << "================================";
    }
}



int main(int argc, char* argv[])
{
    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/log.yml");
    leileilei::ConfigManager::LoadConfigFromYaml(root);
    std::vector<leileilei::Thread::ptr> threads; 
    std::string ss = "name_";
    /**
     * @brief   在无锁的情况下运行fun1函数  最终输出的count会小于等于 100000000
     */
    // for(int i=0;i<10;i++)
    // {
    //     leileilei::Thread::ptr th(new leileilei::Thread(&fun1, ss + std::to_string(i)));
    //     threads.push_back(th);
    // }

    for(int i=0;i<1;i++)
    {
        leileilei::Thread::ptr th(new leileilei::Thread(&fun2, ss + std::to_string(i*2)));
        leileilei::Thread::ptr th2(new leileilei::Thread(&fun3, ss + std::to_string(i*2+1)));

        threads.push_back(th);
        threads.push_back(th2);
    }

    for(size_t i = 0; i<threads.size();i++)
    {
        threads[i]->join();
    }
    LEI_LOG_INFO(logs) << "count = "<< count;
    return 0;
}