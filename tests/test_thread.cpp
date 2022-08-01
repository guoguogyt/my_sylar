#include "leileilei.h"
#include <vector>

int count = 0;

leileilei::Logger::ptr logs = LEI_LOG_GETROOTOR();

void fun1()
{
    for(int i=0;i<100000;i++)
    {
        count++;
    }

}



int main(int argc, char* argv[])
{
    std::vector<leileilei::Thread::ptr> threads; 
    //开线程
    for(int i=0;i<5;i++)
    {
        leileilei::Thread::ptr th(new leileilei::Thread(&fun1, "name_" + std::to_string(i)));
        threads.push_back(th);
    }
    LEI_LOG_INFO(logs) << "count = "<< count;
    return 0;
}