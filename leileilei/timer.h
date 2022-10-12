/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-11 08:51:56
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-12 09:34:59
 */
#pragma once

#include <memory>
#include <set>
#include <vector>
#include "mutex.h"

namespace leileilei
{

class TimerManager;

/**
 * @brief 定时器
 * 
 */
class Timer : public std::enable_shared_from_this<Timer>
{
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

    /**
     * @brief 
     * 取消定时器
     * @return true 
     * @return false 
     */
    bool cancel();
    /**
     * @brief 
     * 刷新设置定时器的执行时间
     * @return true 
     * @return false 
     */
    bool refresh();
    /**
     * @brief 
     * 重新设置定时器的执行周期
     * @param time_period 新的周期 
     * @param from_now 是否从当前的时间开始设置
     * @return true 
     * @return false 
     */
    bool reset(uint64_t time_period, bool from_now);
private:
    /**
     * @brief Construct a new Timer object
     *  构造函数
     * @param tp 执行周期
     * @param sl 是否是循环定时器
     * @param cb 定时器任务
     * @param tm 定时器管理器
     */
    Timer(uint64_t tp, bool sl, std::function<void()> cb, TimerManager* tm);
    /**
     * @brief Construct a new Timer object
     * 构造函数
     * @param nt 定时器执行的精确时间 
     */
    Timer(uint64_t nt);
private:
    // 执行周期
    uint64_t time_period_ = 0;
    // 精确的执行时间
    uint64_t next_time_ = 0;
    // 是否是循环定时器
    bool is_loop_ = false;
    // 定时器任务
    std::function<void()> cb_;
    // 定时器管理器
    TimerManager* tm_;
private:
    struct Comparator
    {
        bool operator()(const Timer::ptr& left, const Timer::ptr& right) const;
    };
};


/**
 * @brief 
 * 定时器管理类，定时器只能在该类中生成, 继承这个类可以使用定时器功能
 */
class TimerManager
{
friend class Timer;
public:
    typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex RWMutexType;
    /**
     * @brief Construct a new Timer Manager object
     * 构造函数
     */
    TimerManager();
    /**
     * @brief Destroy the Timer Manager object
     * 虚的析构函数，因为需要被其他类继承使用
     */
    virtual ~TimerManager();
    /**
     * @brief 
     * 向定时器管理类中添加一个定时器
     * @param time_period 定时器执行周期
     * @param cb 定时器执行函数
     * @param is_loop 是否是循环定时器
     * @return Timer::ptr 
     */
    Timer::ptr addTimer(uint64_t time_period, std::function<void()> cb, bool is_loop = false);
    /**
     * @brief 
     * 向定时器管理类中添加一个条件定时器，只有当条件和时间同时满足时，才会执行定时器
     * @param time_period 定时器周期
     * @param weak_cond 定时器条件
     * @param cb 定时器要执行的任务
     * @param is_loop 是否是循环定时器
     * @return Timer::ptr 
     */
    Timer::ptr addConditionTimer(uint64_t time_period, std::weak_ptr<Timer> weak_cond, std::function<void()> cb, bool is_loop = false);
    /**
     * @brief Get the Next Timer object
     * 返回距离最近一个即将执行的定时器的时间间隔
     * @return uint64_t 
     */
    uint64_t getNextTimer();
    /**
     * @brief Get the Expire Cb object
     * 返回所有过期的定时器的执行任务
     * @param cbs 
     */
    void getExpireCb(std::vector<std::function<void()> >& cbs);
    /**
     * @brief 
     * 是否存在定时器
     * @return true 
     * @return false 
     */
    bool hasTimer();
protected:
    /**
     * @brief 
     * 当一个定时器被加入，且这个定时器是第一个要被执行的任务时，执行该函数，该函数还依赖is_tickle_触发
     */
    virtual void onFrontTimer() = 0;
    /**
     * @brief 向定时器管理器中添加一个定时器
     * @param timer 定时器 
     * @param lock 锁
     */
    void addTimer(Timer::ptr timer, RWMutexType::WriteLock& lock);
private:
    /**
     * @brief 
     * 系统时间是否发生了变化
     * @param now_time 当前的系统时间
     * @return true 
     * @return false 
     */
    bool detectClockRollover(uint64_t now_time);
private:
    // 读写锁
    RWMutexType rwmutex_;
    // 存储定时器
    std::set<Timer::ptr, Timer::Comparator> timer_set_;
    // 最近一次已经执行的定时器时间
    uint64_t last_time_;
    // 是否触发
    bool is_tickle_ = false;
};
    
}