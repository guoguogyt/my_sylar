/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-16 16:21:51
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-28 16:52:22
 */

#include "scheduler.h"
#include "hook.h"

namespace leileilei
{

leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

// 当前线程执行的协程调度器
static thread_local Scheduler* s_schedule = nullptr;
// 当前线程的调度协程
static thread_local Fiber* s_main_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
:scheduler_name_(name)
{
    LEILEILEI_ASSERT(threads > 0);
    // 需要将主线程加入线程池
    if(use_caller)
    {
        Fiber::GetThis();
        threads--;

        LEILEILEI_ASSERT(GetThis() == nullptr);
        s_schedule = this;
        
        rootFiebr_.reset(new Fiber(std::bind(&Scheduler::run,this), 0, true));
        leileilei::Thread::SetThreadName(scheduler_name_);
        s_main_fiber = rootFiebr_.get();
        root_thread_ = leileilei::GetThreadId();
        thread_ids_.push_back(root_thread_);
        LEI_LOG_DEBUG(g_logger) << "main thread join Scheduler thread pool";
    }
    else
    {
        root_thread_ = -1;
    }
    thread_counts_ = threads;
}

Scheduler::~Scheduler()
{
    LEILEILEI_ASSERT(stopping_);
    if(GetThis() == this)
    {
        s_schedule = nullptr;
    }
}

void Scheduler::start()
{
    LEI_LOG_DEBUG(g_logger) << this << "    start Scheduler";
    MutexType::Lock lock(mutex_);
    if(!stopping_)
    {
        return;
    }
    stopping_ = false;// 开始执行状态肯定不是true
    LEILEILEI_ASSERT(thread_pool_.empty());

    thread_pool_.resize(thread_counts_);
    for(int i=0; i<thread_counts_; i++)
    {
        thread_pool_[i].reset(new Thread(std::bind(&Scheduler::run, this), scheduler_name_ + "_" + std::to_string(i)));
        thread_ids_.push_back(thread_pool_[i]->getThreadId());
    }
}

void Scheduler::stop()
{
    LEI_LOG_DEBUG(g_logger) << this << "    try to stop scheduler";
    is_autoStop_ = true;
    // 当线程池中只有一个线程，且这个线程为主线程时，的停止判断
    if(rootFiebr_ && thread_counts_==0 && (rootFiebr_->getState() == Fiber::TERM || rootFiebr_->getState() == Fiber::INIT))
    {
        LEI_LOG_DEBUG(g_logger) << this << "    stopped";
        stopping_ = true;
        if(canStop())   return;
    }
    
    if(root_thread_ != -1)
    {
        LEILEILEI_ASSERT(GetThis() == this);
    }
    else
    {
        LEILEILEI_ASSERT(GetThis() != this)
    }

    stopping_ = true;

    // 为了避免调度器停止时，那些空闲线程正在阻塞中，无法释放，需要通过掉用tickle函数将其唤醒，然后终止线程
    for(int i=0; i<thread_counts_; i++)
    {
        tickle();
    }

    // 主线程在线程池，但是主线程不属于 thread_counts计数中
    if(rootFiebr_)
    {
        tickle();
    }

    // 如果主线程在线程池中,那么主线程的调度协程启动不能在一开始就启动。
    // 如果在一开始就启动那么程序的控制权会进入调度协程，调度协程回去执行Scheduler的run方法，陷入循环，只有当run方法结束时，主线程才会拿到执行权，继续向下执行
    // 这里最后启动了主线程的调度协程，主要是收尾工作。
    if(rootFiebr_)
    {
        if(!canStop())
        {
            LEI_LOG_DEBUG(g_logger) << "还未关闭，需要 启动 主线程的调度协程";
            rootFiebr_->call();
        }
    }

    // 清空线程池
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(mutex_);
        thrs.swap(thread_pool_);
    }

    // 等待所有线程结束
    for(auto& i:thrs)
    {
        i->join();
    }

}

Scheduler* Scheduler::GetThis()
{
    return s_schedule;
}

Fiber* Scheduler::GetMainFiber()
{
    return s_main_fiber;
}

void Scheduler::tickle()
{
    LEI_LOG_DEBUG(g_logger) << "tickle";
}

void Scheduler::run()
{
    LEI_LOG_DEBUG(g_logger) << " begin do Scheduler";
    // 启用hook
    set_hook_enable(true);
    setThis();
    // 产生第一个协程---如果不是主线程，那么线程产生的主协程就是调度协程，主线程则不
    if(leileilei::GetThreadId() != root_thread_)
    {
        s_main_fiber = Fiber::GetThis().get();
    }

    // 产生第二个协程   空跑协程
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    // LEI_LOG_DEBUG(g_logger) << "scheduler run this=" << this;
    // 产生第三个协程   消费协程队列任务的协程
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true)
    {
        ft.reset(); // 每次清空数据
        bool is_active = false;
        bool tickle_me = false;
        {
            // 取数据
            MutexType::Lock lock(mutex_);
            LEI_LOG_DEBUG(g_logger) << "fiber list size" << fiber_list_.size();
            auto it = fiber_list_.begin();
            while(it != fiber_list_.end())
            {
                // 该线程是否可以执行
                if(it->thread_ != -1 && it->thread_ != leileilei::GetThreadId())
                {
                    ++it;
                    tickle_me = true;
                    continue;
                }
                LEILEILEI_ASSERT(it->fiber_ || it->cb_);
                // 如果协程正在执行，就放掉
                if(it->fiber_ && it->fiber_->getState() == Fiber::EXEC)
                {
                    ++it;
                    continue;
                }
                // 取出数据
                ft = *it;
                fiber_list_.erase(it++);
                active_thread_count_++;
                is_active = true;
                break;
            }
        }

        if(tickle_me)
        {
            tickle();
        }

        // 处理协程
        if(ft.fiber_ && ft.fiber_->getState() != Fiber::TERM && ft.fiber_->getState() != Fiber::EXCEPT)
        {
            LEI_LOG_DEBUG(g_logger) << "====================fiber task====================";
            // 开始执行协程任务
            ft.fiber_->swapIn();

            active_thread_count_--;

            // 执行结束之后如果是ready在放入协程任务队列
            if(ft.fiber_->getState() == Fiber::READY)
            {
                schedule(ft.fiber_);
            }
            else if(ft.fiber_->getState() != Fiber::TERM && ft.fiber_->getState() != Fiber::EXCEPT)
            {
                ft.fiber_->state_ = Fiber::HOLD;
            }
            ft.reset();
        }
        // 处理函数
        else if(ft.cb_)
        {
            LEI_LOG_DEBUG(g_logger) << "++++++++++++++++++++cb task+++++++++++++++++++";
            // 如果cb_fiber已经存在, 直接替换绑定函数即可
            if(cb_fiber)
            {
                cb_fiber->reset(ft.cb_);
            }
            // 否则需要生成新的协程
            else
            {
                cb_fiber.reset(new Fiber(ft.cb_));
            }
            ft.reset();
            cb_fiber->swapIn();
            active_thread_count_--;
            if(cb_fiber->getState() == Fiber::READY)
            {
                schedule(cb_fiber);
                cb_fiber.reset();
            }
            else if(cb_fiber->getState() == Fiber::TERM || cb_fiber->getState() == Fiber::EXCEPT)
            {
                cb_fiber->reset(nullptr);
            }
            else
            {
                cb_fiber->state_ = Fiber::HOLD;
                cb_fiber.reset();
            }
        }
        else// 空跑
        {
            if(is_active)
            {
                active_thread_count_--;
                continue;
            }

            // 如果空跑也执行结束那么就退出
            if(idle_fiber->getState() == Fiber::TERM)
            {
                LEI_LOG_DEBUG(g_logger) << "idle fiber trem";
                break;
            }

            idle_thread_count_++;
            idle_fiber->swapIn();
            // LEI_LOG_DEBUG(g_logger) << "out idle func";
            idle_thread_count_--;

            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT)
            {
                idle_fiber->state_ = Fiber::HOLD;
            }
        }
        
    }
}

// 不执行stop函数，这个返回值永远是0， 及时执行了stop函数还需要看其他条件
bool Scheduler::canStop()
{
    // LEI_LOG_DEBUG(g_logger) << "is_autoStop_[" << is_autoStop_ <<"]     stopping_[" << stopping_
    //                         << "]   fiber_list_ empty[" << fiber_list_.empty() << "] active_thread_count_["
    //                         << active_thread_count_ << "]";
    return is_autoStop_ && stopping_ && fiber_list_.empty() && active_thread_count_==0;
}

void Scheduler::idle()
{
    LEI_LOG_DEBUG(g_logger) << "idle";
    while(!canStop())
    {
        Fiber::YieldToHold();
    }
}

void Scheduler::setThis()
{
    s_schedule = this;
}


}

