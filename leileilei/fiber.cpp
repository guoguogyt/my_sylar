/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-08-22 15:33:45
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-13 15:48:37
 */
#include "fiber.h"
#include "log.h"
#include "config.h"
#include "macro.h"
#include <atomic>

namespace leileilei
{

// system日志
static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

// 协程id
static std::atomic<uint64_t> s_fiber_id {0}; 
// 协程数量
static std::atomic<uint64_t> s_fiber_count {0}; 

// 当前正在执行的协程
static thread_local Fiber* t_fiber = nullptr;
// 表示线程的主协程
static thread_local Fiber::ptr t_threadFiber = nullptr;

// 设置默认的栈大小，可以从配置文件中载入
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = ConfigManager::LookUp<uint32_t>("fiber.stack_size", 128*1024, "fiber stack size");

// 封装在堆上申请内存的类
class  MallocStackAllocator
{
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size)
    {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

/**
 * @brief Construct a new Fiber:: Fiber object
 * 只用于生成主协程. 通过MainFiber函数调用
 */
Fiber::Fiber()
{   
    state_ = EXEC;
    SetThis(this);

    if(getcontext(&ctx_))   LEILEILEI_ASSERT2(false, "getcontext error");

    s_fiber_count++;

    LEI_LOG_DEBUG(g_logger) << "Fiber[is fiber main]";
}   

/**
 * @brief Construct a new Fiber:: Fiber object
 *  生成真正去执行任务的协程
 * @param cb 
 * @param stacksize 
 * @param use_caller 
 */
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
{
    s_fiber_count++; // 协程总数加1
    id_ = ++s_fiber_id;   //  协程id加1

    callback_ = cb;
    
    // 栈
    stacksize_ = stacksize ? stacksize : g_fiber_stack_size->getValue();
    stack_ = StackAllocator::Alloc(stacksize_);

    // 获取协程上下文
    if(getcontext(&ctx_))   LEILEILEI_ASSERT2(false, "getcontext error");

    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stacksize_;

    // 为这个协程绑定一个函数
    makecontext(&ctx_, &Fiber::MainFunc, 0);

    LEI_LOG_DEBUG(g_logger) << "Fiber::sub Fiber id = " << id_;    
}

Fiber::~Fiber()
{
    s_fiber_count--;
    // stack_不为空，说明不是主协程
    if(stack_)
    {
        // 检查状态
        LEILEILEI_ASSERT(state_ == TERM || state_ == EXCEPT || state_ == INIT)
        StackAllocator::Dealloc(stack_, stacksize_);
    }
    else
    {
        LEILEILEI_ASSERT(!callback_);
        LEILEILEI_ASSERT(state_ == EXEC);

        Fiber* cur_fiber = t_fiber;
        if(cur_fiber == this)
        {
            SetThis(this);
        }
    }
    LEI_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << id_
                            << "    total=" << s_fiber_count;
}

void Fiber::reset(std::function<void()> cb)
{
    LEILEILEI_ASSERT(state_ == TERM || state_ == INIT || state_ == EXCEPT);
    LEILEILEI_ASSERT(!cb);

    callback_ = cb;

    // 重新获取协程的上下文
    if(getcontext(&ctx_))  LEILEILEI_ASSERT2(false, "getcontext error");
    
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stacksize_;

    makecontext(&ctx_, &Fiber::MainFunc, 0);

    state_ = INIT;
}

void Fiber::swapIn()
{
    SetThis(this);
    LEILEILEI_ASSERT(state_ != EXEC);
    state_ = EXEC;
    if(swapcontext(&t_threadFiber->ctx_, &ctx_))    LEILEILEI_ASSERT2(false, "swapcontext error");
}

void Fiber::swapOut()
{
    SetThis(t_threadFiber.get());
    if(swapcontext(&ctx_, &t_threadFiber->ctx_))    LEILEILEI_ASSERT2(false, "swapcontext error");
}

uint64_t Fiber::getId()
{
    return id_;
}

void Fiber::SetThis(Fiber* f)
{
    t_fiber = f;
}

/**
 * @brief 
 * 
 * @return Fiber::ptr 
 */
Fiber::ptr Fiber::GetThis()
{
    if(t_fiber) return t_fiber->shared_from_this();

    // 生成主协程
    Fiber::ptr main_fiber(new Fiber);
    LEILEILEI_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return main_fiber->shared_from_this();
}

void Fiber::YieldToReady()
{
    Fiber::ptr cur_fiber = GetThis();
    LEILEILEI_ASSERT(cur_fiber->state_ == EXEC);
    cur_fiber->state_ = READY;
    cur_fiber->swapOut();
}

void Fiber::YieldToHold()
{
    Fiber::ptr cur_fiber = GetThis();
    LEILEILEI_ASSERT(cur_fiber->state_ == EXEC);
    cur_fiber->swapOut();
}

uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}

/**
 * @brief 执行协程绑定的函数
智能指针所指向的对象的析构问题：
    程序执行结束，申请的所有资源都会被释放，但是和执行析构是两回事情。
    析构是程序运行中对象的自然销毁，程序都已经结束，那么不会在继续执行代码
 * 
 */
void Fiber::MainFunc()
{
    // 当函数执行结束，需要将协程控制权返回到主协程(因为在子协程创建的时候，并没有指定uc_link的下文)
    Fiber::ptr cur_fiber = Fiber::GetThis();
    try
    {
        cur_fiber->callback_();
        cur_fiber->callback_ = nullptr;
        cur_fiber->state_ = TERM;
    }catch(std::exception& ex)
    {
        cur_fiber->state_ = EXCEPT;
        LEI_LOG_ERROR(g_logger) << "Fiber Except " << ex.what()
                                << "fiber id = " << cur_fiber->getId()
                                << std::endl
                                << leileilei::BacktraceToString();
    }catch(...)
    {
        cur_fiber->state_ = EXCEPT;
        LEI_LOG_ERROR(g_logger) << "Fiber Except "
                                << "fiber id = " << cur_fiber->getId()
                                << std::endl
                                << leileilei::BacktraceToString();
    }

    // 为了避免sub fiber的智能指针引用计数到不了0，导致协程无法析构
    // 这里手工reset使得，智能指针的引用计数-1
    // 因为cur_fiber无法直接完整个函数的代码块(被swapOut出去了,所以代码不会执行完)
    auto temp = cur_fiber.get();
    cur_fiber.reset();
    temp->swapOut();

    LEILEILEI_ASSERT2(false, "fiber shoud not reach there, fiberid=" + std::to_string(temp->getId()));
}

uint64_t Fiber::GetFiberId()
{
    if(t_fiber) t_fiber->getId();

    return 0;
}

}