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

    for(size_t i = 0; i<threads.size();i++)
    {
        threads[i]->join();
    }
    LEI_LOG_INFO(logs) << "count = "<< count;
    return 0;
}