#pragma once

#include "mutex.h"

namespace leileilei
{

/**
 * @brief 线程类 
 *  执行构造函数时创建线程
 *  构造函数参数传入线程要执行的函数与函数名称
 */
class Thread
{
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb , std::string& name);

    ~Thread();

    /**
     * @brief Get the Thread Id object
     *  得到线程id
     * @return pid_t 
     */
    pid_t getThreadId() {   return thread_id_;}

    /**
     * @brief Get the Thread Name object
     *  得到线程名称
     * @return const std::string& 
     */
    const std::string& getThreadName() const {  return thread_name_;}

    /**
     * @brief   等待线程执行完成 
     */
    void join();

    /**
     * @brief Get the This object
     *  获取当前的线程指针
     * @return Thread* 
     */
    static Thread* getThis();

    /**
     * @brief Get the Thread Name object
     *  获取当前线程的名称
     * @return const std::string& 
     */
    static const std::string& GetThreadName();

    /**
     * @brief Set the Thread Name object
     *  设置当前线程的名称
     * @param name 
     */
    static void SetThreadName(std::string& name);

private:
    /**
     * @brief   线程运行函数，内部会执行绑定的函数 
     * @param arg 
     * @return void* 
     */
    static void* run(void* arg);

private:
    //线程id
    pid_t thread_id_ = 0;
    //线程
    pthread_t thread_ = 0;
    //绑定的函数
    std::function<void()> callback_ = nullptr;
    //线程名称
    std::string thread_name_;
    //信号量


};



}

