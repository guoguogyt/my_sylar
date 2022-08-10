/**
 * @file mutex.h
 * @author your name (you@domain.com)
 * @brief   封装 信号量、互斥量、一些锁
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <functional>
#include <memory>
#include <pthread.h>
#include <thread>
#include <semaphore.h>
#include <stdexcept>

namespace leileilei
{

/**
 * @brief 信号量
 * 
 */
class Semaphore
{
public:
    /**
     * @brief Construct a new Semaphore object
     *  构造函数，参数是信号量值得大小
     * @param count 
     */
    Semaphore(uint32_t count = 0);
    /**
     * @brief Destroy the Semaphore object
     *  析构函数
     */
    ~Semaphore();
    /**
     * @brief 获取信号量
     */
    void wait();
    /**
     * @brief   释放信号量 
     */
    void notify();
private:
    //信号量
    sem_t semaphore_;
};

/**
 * @brief 
 *  锁的模板类，真正被使用的类
 * @tparam T 
 */
template<class T>
struct LockImpl
{
public:
    /**
     * @brief Construct a new Lock Impl object
     *  构造函数加锁
     * @param mutex 
     */
    LockImpl(T& mutex):mutex_(mutex)
    {
        mutex_.lock();
        islock_ = true;
    }
    /**
     * @brief Destroy the Lock Impl object
     *  析构函数解锁
     */
    ~LockImpl()
    {
        unlock();
    }
    /**
     * @brief 
     *  调用的是传入类的加锁函数
     */
    void lock()
    {
        if(!islock_)
            mutex_.lock();
            islock_ = true;
    }
    /**
     * @brief 
     *  解锁
     */
    void unlock()
    {
        if(islock_)
        {
            mutex_.unlock();
            islock_ = false;
        }
    }
private:
    T& mutex_;
    //是否已经上锁
    bool islock_;
};

/**
 * @brief 互斥量
 * 
 */
class Mutex{
public:
    typedef LockImpl<Mutex> Lock;

    /**
     * @brief Construct a new Mutex object
     #include <pthread.h>
        int pthread_mutex_init(pthread_mutex_t *restrict mutex,const pthread_mutexattr_t *restrict attr);
        pthread_mutex_init() 函数是以动态方式创建互斥锁的，参数attr指定了新建互斥锁的属性。如果参数attr为空(NULL)，则使用默认的互斥锁属性，默认属性为快速互斥锁 。
        pthread_mutexattr_init() 函数成功完成之后会返回零，其他任何返回值都表示出现了错误。
     *  构造函数
     */
    Mutex()
    {
        pthread_mutex_init(&mutex_, nullptr);
    }
    /**
     * @brief Destroy the Mutex object
     *  析构函数
     */
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }
    /**
     * @brief 
     *  加锁
     */
    void lock()
    {
        pthread_mutex_lock(&mutex_);
    }
    /**
     * @brief 
     *  解锁
     */
    void unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
};


/**
 * @brief   自旋锁 
 自旋锁适用于那些仅需要阻塞很短时间的场景

 自旋锁是 SMP 架构中的一种 low-level 的同步机制。
当线程A想要获取一把自旋锁而该锁又被其它线程锁持有时，线程A会在一个循环中自旋以检测锁是不是已经可用了。

对于使用自选锁需要注意：

由于自旋时不释放CPU，因而持有自旋锁的线程应该 尽快释放自旋锁 ，否则等待该自旋锁的线程会一直在那里自旋，这就会浪费CPU时间。
持有自旋锁的线程 在sleep之前应该释放自旋锁 以便其它线程可以获得自旋锁，或者拿到自旋锁之后不允许睡眠。
参与自旋锁竞争的两个或多个线程如果绑定在同一个CPU核上，则其调度属性设置为实时调度（RT_FIFO）时必须将其优先级设置相同，否则自旋锁无法在线程之间切换，这是因为低优先级任务拿到锁之后被切出去之后，高优级任务会一直在那自旋拿锁会将CPU核打满，导致低优先级任务一直因得不到执行而不能放锁。
同互斥锁相同用法顺序：拿锁、访问公共资源、放锁。特别强调的是 拿锁之后代码中的异常分支处理中必须放锁。
自旋锁不支持递归调用，线程重复拿锁时会发生自旋
使用任何锁需要消耗系统资源（内存资源和CPU时间），这种资源消耗可以分为两类：

建立锁所需要的资源
线程被阻塞时锁所需要的资源
 */
class SpinLock
{
public:
    typedef LockImpl<SpinLock> Lock;

    /**
     * @brief Construct a new Spin Lock object
     * 构造
     */
    SpinLock()
    {
        pthread_spin_init(&mutex_, 0);
    }
    /**
     * @brief Destroy the Spin Lock object
     *  析构
     */
    ~SpinLock()
    {
        pthread_spin_destroy(&mutex_);
    }
    /**
     * @brief 竞争锁
     * 
     */
    void lock()
    {
        pthread_spin_lock(&mutex_);
    }
    /**
     * @brief 释放锁
     * 
     */
    void unlock()
    {
        pthread_spin_unlock(&mutex_);
    }

private:
    // 自旋锁
    pthread_spinlock_t mutex_;
};

/**
 * @brief 原子锁
 *  
 */
class CASLock
{
public:
    /// 局部锁
    typedef LockImpl<CASLock> Lock;

    /**
     * @brief 构造函数
     */
    CASLock() {
        mutex_.clear();
    }

    /**
     * @brief 析构函数
     */
    ~CASLock() {
    }

    /**
     * @brief 上锁
     */
    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&mutex_, std::memory_order_acquire));
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        std::atomic_flag_clear_explicit(&mutex_, std::memory_order_release);
    }
private:
    // 原子状态
    volatile std::atomic_flag mutex_;
};

}