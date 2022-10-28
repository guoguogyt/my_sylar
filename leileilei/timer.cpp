/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-11 08:52:03
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-28 11:46:37
 */

#include "timer.h"
#include "util.h"

namespace leileilei
{

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

bool Timer::cancel()
{
    TimerManager::RWMutexType::WriteLock lock(tm_->rwmutex_);
    if(cb_)
    {
        cb_ = nullptr;
        auto it = tm_->timer_set_.find(shared_from_this());
        // if(it == tm_->timer_set_.begin() && !is_tickle_)
        // {
        //     onFrontTimer();
        // }
        tm_->timer_set_.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh()
{
    TimerManager::RWMutexType::WriteLock lock(tm_->rwmutex_);
    if(!cb_)
    {
        return false;
    }
    auto it = tm_->timer_set_.find(shared_from_this());
    if(it == tm_->timer_set_.end())
    {
        return false;
    }
    tm_->timer_set_.erase(it);
    next_time_ = leileilei::GetCurrentMS() + time_period_;
    tm_->addTimer(shared_from_this(), lock);
    return true;

}

bool Timer::reset(uint64_t time_period, bool from_now)
{
    TimerManager::RWMutexType::WriteLock lock(tm_->rwmutex_);
    if(!cb_)
    {
        return false;
    }
    auto it = tm_->timer_set_.find(shared_from_this());
    if(it == tm_->timer_set_.end())
    {
        return false;
    }
    tm_->timer_set_.erase(it);
    next_time_ = from_now ? (leileilei::GetCurrentMS() + time_period) : (next_time_ - time_period_ + time_period);
    time_period_ = time_period;
    tm_->addTimer(shared_from_this(), lock);
    return true;
}

Timer::Timer(uint64_t tp, bool sl, std::function<void()> cb, TimerManager* tm)
    :time_period_(tp), is_loop_(sl), cb_(cb), tm_(tm)
{
    next_time_ = leileilei::GetCurrentMS() + time_period_;
}

Timer::Timer(uint64_t nt)
    :next_time_(nt)
{

}

bool Timer::Comparator::operator()(const Timer::ptr& left, const Timer::ptr& right) const
{
    if(!left && !right) return false;
    if(!left) return true;
    if(!right) return false;
    if(left->next_time_ < right->next_time_)    return true;
    if(left->next_time_ > right->next_time_)    return false;
    return left.get() < right.get();
}


TimerManager::TimerManager()
{
    last_time_ = leileilei::GetCurrentMS();
}

TimerManager::~TimerManager()
{

}

Timer::ptr TimerManager::addTimer(uint64_t time_period, std::function<void()> cb, bool is_loop)
{
    Timer::ptr timer(new Timer(time_period, is_loop, cb, this));
    RWMutexType::WriteLock lock(rwmutex_);
    addTimer(timer, lock);
    return timer;
}

static void OnTimer(std::weak_ptr<Timer> weak_cond, std::function<void()> cb)
{
    std::shared_ptr<void> temp = weak_cond.lock();
    if(temp)
    {
        cb();
    }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t time_period, std::weak_ptr<Timer> weak_cond, std::function<void()> cb, bool is_loop)
{
    return addTimer(time_period, std::bind(&OnTimer, weak_cond, cb), is_loop);
}

uint64_t TimerManager::getNextTimer()
{
    RWMutexType::ReadLock lock(rwmutex_);
    is_tickle_ = false;
    if(!hasTimer()) return ~0ull;

    const Timer::ptr& f = *timer_set_.begin();
    uint64_t cur_time = leileilei::GetCurrentMS();
    if(cur_time > f->next_time_)    return 0;
    return f->next_time_ - cur_time;
}

void TimerManager::getExpireCb(std::vector<std::function<void()> >& cbs)
{
    uint64_t cur_time = leileilei::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(rwmutex_);
        if(timer_set_.empty())  return;
    }

    RWMutexType::WriteLock lock(rwmutex_);
    bool rollover = detectClockRollover(cur_time);

    if(!rollover && (*timer_set_.begin())->next_time_ > cur_time)    return;
    Timer::ptr ptr(new Timer(cur_time));
    auto it = rollover ? timer_set_.end() : timer_set_.lower_bound(ptr);
    while(it != timer_set_.end() && (*it)->next_time_ == cur_time)
    {
        it++;
    }
    for(auto& time : timer_set_)
    {
        LEI_LOG_DEBUG(g_logger) << "time next_time" << time->next_time_;
    }
    expired.insert(expired.begin(), timer_set_.begin(), it);
    LEI_FMt_LOG_DEBUG(g_logger, "rollover[%d]    expired size[%d]    timer_set_ size[%d]", rollover, expired.size(), timer_set_.size());
    timer_set_.erase(timer_set_.begin(), it);
    cbs.resize(expired.size());
    for(auto& time : expired)
    {
        cbs.push_back(time->cb_);
        LEI_FMt_LOG_DEBUG(g_logger, "timer_time[%d]     cur_time[%d]", time->next_time_, cur_time);
        if(time->is_loop_)
        {
            time->next_time_ = cur_time + time->time_period_;
            // 这里直接插入，如果插入的是第一个不会唤醒onFrontTimer()函数
            timer_set_.insert(time);
        }
        else
        {
            time->cb_ = nullptr;
        }
    }
    LEI_LOG_DEBUG(g_logger) << "cbs size = "<< cbs.size();
}

bool TimerManager::hasTimer()
{
    RWMutexType::ReadLock lock(rwmutex_);
    return !timer_set_.empty();
}

void TimerManager::addTimer(Timer::ptr timer, RWMutexType::WriteLock& lock)
{
    auto it = timer_set_.insert(timer).first;
    bool f = (it == timer_set_.begin()) && !is_tickle_;
    if(f)   is_tickle_ = true;
    lock.unlock();
    if(f)   onFrontTimer();
}

bool TimerManager::detectClockRollover(uint64_t now_time)
{
    bool rollover = false;
    if(now_time < last_time_ && now_time < (last_time_ - 60*60*1000))
    {
        rollover = true;
    }
    last_time_ = now_time;
    return rollover;
}



}