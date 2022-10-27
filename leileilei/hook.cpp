/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-26 16:13:03
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-27 15:27:35
 */

#include <dlfcn.h>
#include "hook.h"
#include "iomanager.h"
#include "fiber.h"

leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

namespace leileilei
{

static thread_local bool t_hook_enable = false;

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
/**
 * @brief 
 * dlsym    使用dlsym要引入 dl动态库
 */
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


bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}
}


extern "C"
{

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds)
{
    if(!leileilei::is_hook_enable())
    {
        return sleep_f(seconds);
    }
    LEI_LOG_DEBUG(g_logger) << "do hook!";
    // 获取当前正常执行的协程
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    /**
     * 这里想绑定的其实是Scheduler::schedule函数
     但是iom是 IOManager类型
     */
    iom->addTimer(seconds * 1000, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec)
{
    if(!leileilei::is_hook_enable())
    {
        return usleep_f(usec);
    }
    // 获取当前正常执行的协程
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;   
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    if(!leileilei::is_hook_enable())
    {
        return nanosleep_f(req, rem);
    }
    // 获取当前正常执行的协程
    int timeout_ns = req->tv_sec*1000 + req->tv_nsec / 1000 / 1000;
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    iom->addTimer(timeout_ns, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;   
}

}
