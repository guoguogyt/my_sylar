/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-26 16:13:03
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-27 10:48:09
 */
#include "hook.h"
#include "iomanager.h"
#include "fiber.h"

namespace leileilei
{

static thread_local bool t_hook_enable = false;

bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep)

// 用于初始化
void for_hook_init()
{
    static bool is_init = false;
    if(is_init)
    {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}
// 用于初始化
struct _HookInit
{
    _HookInit()
    {
        for_hook_init();
    }
};
// 用于初始化   静态全局变量的构造函数在main函数执行之前执行
static _HookInit __init__;

}


extern "C"
{
unsigned int sleep(unsigned int seconds)
{
    if(!leileilei::is_hook_enable())
    {
        return sleep_f(seconds);
    }
    // 获取当前正常执行的协程
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    // iom->addTimer(seconds*1000, std::bind(&leileilei::Scheduler::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec)
{
    return 0;    
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    return 0;
}

}
