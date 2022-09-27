/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-16 16:21:28
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-27 16:41:26
 */
#pragma once

#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <atomic>
#include "thread.h"
#include "fiber.h"
#include "log.h"
#include "macro.h"

#define MAX_FIBER_LIST_SIZE 1000000

namespace leileilei
{

/**
 * @brief 
 协程调度类
 1、含有线程池
 2、含有协程(协程或协程执行函数)队列

 目标:
    多个线程竞争消费协程队列里面的信息
    线程通过创建"主协程"(如果是主线程也在线程池则不用主协程)进行线程调度
    也可以将某个任务，指定给某个线程执行
注意:
    线程池可以将主线程放入，放入主线程之后，情况会变的复杂

增加：
    协程队列大小限制,不能无限制的往协程队列里面放任务
    当关闭调度器的过程中，不允许在向协程队列中push任务
 */
class Scheduler
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;
public:
    /**
     * @brief Construct a new Scheduler object
     *  
     * @param threads   线程池中的线程数量 
     * @param use_caller   是否将主线程也放入线程池
     * @param name  调度器名称
     */
    Scheduler(size_t threads = 1, bool use_caller = false, const std::string& name = "");

    /**
     * @brief Destroy the Scheduler object
     * 
     */
    virtual ~Scheduler();

    /**
     * @brief Get the Name object
     *  返回调度器名称
     * @return const std::string& 
     */
    const std::string& getName() const {    return scheduler_name_;}

    /**
     * @brief 
     *  启动协程调度器
     */
    void start();

    /**
     * @brief 
     * 关闭协程调度器
     */
    void stop();

    /**
     * @brief 
     *  将一个协程任务添加进入协程队列
     * @tparam FiberOrCb 泛型（调用过程中会传入Fiber或者函数类型）
     * @param fc   一个任务
     * @param thread 指定要被哪个线程运行，如果为-1时，则任意的一个活跃线程都可以运行该任务
     */
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1)
    {
        if(canStop())
        {
            LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << " scheduler is stopping, can not add fiber task";
            return;
        }
        bool need_tickle = false;
        {
            MutexType::Lock lock(mutex_);
            if(fiber_list_.size() >= MAX_FIBER_LIST_SIZE)
            {
                LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << " fiber list is full, can not add fiber task";
                return;
            }
            need_tickle = scheduleNoLock(fc, thread);
        }
        // 需要唤醒
        if(need_tickle)
        {
            tickle();
        }
    }

    /**
     * @brief 
     *  批量将协程任务添加进入协程队列,但是无法指定让某个特定的线程去运行
     * @tparam FiberOrCbIterator 
     * @param begin 
     * @param end 
     */
    template<class FiberOrCbIterator>
    void schedule(FiberOrCbIterator begin, FiberOrCbIterator end)
    {
        if(canStop())
        {
            LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << " scheduler is stopping, can not add fiber task";
            return;
        }
        bool need_tickle = false;
        {
            MutexType::Lock lock(mutex_);
            while(begin != end)
            {
                if(fiber_list_.size() >= MAX_FIBER_LIST_SIZE)
                {
                    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << " fiber list is full, can not add fiber task";
                    return;
                }
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                begin++;
            }
        }
        if(need_tickle)
        {
            tickle();
        }
    }

public:
    /**
     * @brief Get the This object
     *  返回当前运行的协程调度器
     * @return Scheduler* 
     */
    static Scheduler* GetThis();
    
    /**
     * @brief Get the Main Fiber object
     *  返回  正在运行线程的调度协程
     * @return Fiber* 
     */
    static Fiber* GetMainFiber();

protected:
    /**
     * @brief 
     *  通知协程调度器有任务了
     */
    virtual void tickle();

    /**
     * @brief 
     * 协程调度函数(如何将协程调度给线程)
     */
    void run();

    /**
     * @brief 
     * 返回是否可以停止
     * @return true 
     * @return false 
     */
    virtual bool canStop();

    /**
     * @brief 
     * 当协程无任务执行时，执行idle协程任务
     */
    virtual void idle();

    /**
     * @brief Set the This object
     *  设置当前的调度器
     */
    void setThis();

    /**
     * @brief 
     *  当前是否有空闲的线程
     * @return true 
     * @return false 
     */
    bool hasIdleThreads() { return idle_thread_count_ > 0;}

private:
    /**
     * @brief 
     * 将协程任务放入到协程队列中
     * @tparam FiberOrCb 
     * @param fc 协程任务
     * @param thread 将当前任务分配给哪个线程
     * @return true 通知调度器
     * @return false 不需要通知调度器
     */
    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread)
    {
        bool need_tickle = fiber_list_.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber_ || ft.cb_)
        {
            fiber_list_.push_back(ft);
        }
        return need_tickle;
    }

private:
    /**
     * @brief 
     * 协程/函数/线程组
     */
    struct FiberAndThread
    {
        // 协程
        Fiber::ptr fiber_;
        // 执行函数
        std::function<void()> cb_;
        // 运行线程
        int thread_;

        FiberAndThread(Fiber::ptr fiber, int id):fiber_(fiber), thread_(id) {}
        FiberAndThread(Fiber::ptr* fiber, int id):thread_(id)    {   fiber_.swap(*fiber);}
        FiberAndThread(std::function<void()> callback, int id):cb_(callback),thread_(id)  {}
        FiberAndThread(std::function<void()>* callback, int id):thread_(id) {   cb_.swap(*callback);}
        FiberAndThread():thread_(-1)    {}
        /**
         * @brief 
         * 重置所有属性
         */
        void reset()
        {
            fiber_ = nullptr;
            cb_ = nullptr;
            thread_ = -1;
        }
    };

private:
    // 锁 
    MutexType mutex_;
    // 线程池
    std::vector<Thread::ptr> thread_pool_;
    // 协程任务队列
    std::list<FiberAndThread> fiber_list_;
    // 如果将主线程加入线程池,则会有值,且为调度协程
    Fiber::ptr rootFiebr_;
    // 调度器名称
    std::string scheduler_name_;

protected:
    // 线程池中的id
    std::vector<int> thread_ids_;
    // 线程池中线程数量
    int thread_counts_ = 0;
    // 活跃线程数量
    std::atomic<int> active_thread_count_ = {0};
    // 空闲线程数量
    std::atomic<int> idle_thread_count_ = {0};
    // 是否正在停止
    bool stopping_ = true;
    // 是否自动停止
    bool is_autoStop_ = false;
    // 如果将主线程加入线程池,则会有值,且为主线程
    int root_thread_ = 0;
};

}