#include  <stdexcept>
#include "thread.h"
#include "log.h"

namespace leileilei
{
/**
 * @brief 
 * 线程局部存储 thread_local
 每个线程都拥有该变量的一份拷贝，且互不干扰
 线程局部存储中的变量将一直存在，直至线程终止，当线程终止时会自动释放这一存储
 主要适用的场景是：本线程这个生命周期里面修改和读取，不会与别的线程相互影响。
 */
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static Logger::ptr thread_log = LEI_GET_LOGGER("system");


Thread::Thread(std::function<void()> cb, std::string& name)
    :callback_(cb),
    thread_name_(name)
{
    if(name.empty())    thread_name_ = "UNKNOW";

    int result = pthread_create(&thread_, nullptr, &Thread::run, this);
    if(result)
    {
        LEI_LOG_ERROR(thread_log) << "thread create faild, result="<< result << "   name=" << thread_name_;
        throw std::logic_error("pthread create error");
    }

}

Thread::~Thread()
{
    if(thread_)
    {
        pthread_detach(thread_);
    }
}

void Thread::join()
{
    if(thread_)
    {
        //第二个参数可以获取线程返回的值
        int result = pthread_join(thread_, nullptr);
        if(result)
        {
            LEI_LOG_ERROR(thread_log) << "pthread_join faild , result=" <<result;
            throw std::logic_error("pthread join error");
        }
        //等待结束置0
        thread_ = 0;
    }
}

void* Thread::run(void* arg)
{
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->thread_name_;
    thread->thread_id_ = leileilei::GetThreadId();

    std::function<void()> cb;
    cb.swap(thread->callback_);

    cb();

    return 0;    
}

Thread* Thread::getThis()
{
    return t_thread;
}

const std::string& Thread::GetThreadName()
{
    return t_thread_name;
}

void Thread::SetThreadName(std::string& name)
{
    if(name.empty())
        return;
    if(t_thread)
    {
        t_thread->thread_name_ = name;
    }
    t_thread_name = name;
}

}

