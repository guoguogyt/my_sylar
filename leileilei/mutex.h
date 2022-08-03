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
#include  <stdexcept>

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

}