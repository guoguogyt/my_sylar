#include "leileilei.h"
#include <vector>
#include <unistd.h>

int count = 0;

leileilei::Logger::ptr logs = LEI_LOG_GETROOTOR();

void fun1()
{
    LEI_LOG_INFO(logs) << "name: " << leileilei::Thread::GetThreadName()
                             << " this.name: " << leileilei::Thread::getThis()->getThreadName()
                             << " id: " << leileilei::GetThreadId()
                             << " this.id: " << leileilei::Thread::getThis()->getThreadId();
    for(int i=0;i<1000000;i++)
    {
        ++count;
    }
    // sleep(20);
}



int main(int argc, char* argv[])
{
    std::vector<leileilei::Thread::ptr> threads; 
    std::string ss = "name_";
    //开线程
    for(int i=0;i<5;i++)
    {
        // leileilei::Thread::ptr th(new leileilei::Thread(&fun1, "name_" + std::to_string(i)));
        leileilei::Thread::ptr th(new leileilei::Thread(&fun1, ss + std::to_string(i)));
        threads.push_back(th);
    }

    for(size_t i = 0; i<threads.size();i++)
    {
        threads[i]->join();
    }
    LEI_LOG_INFO(logs) << "count = "<< count;
    return 0;
}