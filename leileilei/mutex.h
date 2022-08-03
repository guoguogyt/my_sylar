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
    T& muext_;
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


}