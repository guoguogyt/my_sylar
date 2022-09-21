/**
 * @file fiber.h
 * @author your name (you@domain.com)
 * @brief 协程是一种用户态的轻量级线程，协程的调度完全由用户控制(用户态)
     一个线程可以拥有多个协程，协程不是被操作系统内核所管理，而完全是由程序所控制

子程序，或者称为函数，在所有语言中都是层级调用，比如A调用B，B在执行过程中又调用了C，C执行完毕返回，B执行完毕返回，最后是A执行完毕。
所以子程序调用是通过栈实现的，一个线程就是执行一个子程序。
子程序调用总是一个入口，一次返回，调用顺序是明确的。而协程的调用和子程序不同。
协程看上去也是子程序，但执行过程中，在子程序内部可中断，然后转而执行别的子程序，在适当的时候再返回来接着执行。
注意，在一个子程序中中断，去执行其他子程序，不是函数调用，有点类似CPU的中断。

协程的特点在于是一个线程执行，最大的优势就是协程极高的执行效率。
因为子程序切换不是线程切换，而是由程序自身控制，因此，没有线程切换的开销，和多线程比，线程数量越多，协程的性能优势就越明显。
不需要多线程的锁机制，因为只有一个线程，也不存在同时写变量冲突，在协程中控制共享资源不加锁，只需要判断状态就好了，所以执行效率比多线程高很多。

因为协程是一个线程执行，那怎么利用多核CPU呢？最简单的方法是多进程+协程，既充分利用多核，又充分发挥协程的高效率，可获得极高的性能。

协程的关键特点是调度/挂起可以由开发者控制。协程比线程轻量的多。
在语言层面实现协程是让其内部有一个类似栈的数据结构，当该协程被挂起时能够保存该协程的数据现场以便恢复执行。 
在汇编层面理解，协程无非就是jump而已，只不过要把当前的环境保存。流程大致是保存当前的环境，然后跳转到另一个环境中。执行完再跳回来，在保存的环境中继续执行。



 下面详细介绍四个函数：
     int getcontext(ucontext_t *ucp);
 初始化ucp结构体，将当前的上下文保存到ucp中
     int setcontext(const ucontext_t *ucp);
 设置当前的上下文为ucp，setcontext的上下文ucp应该通过getcontext或者makecontext取得，如果调用成功则不返回。如果上下文是通过调用getcontext()取得,程序会继续执行这个调用。如果上下文是通过调用makecontext取得,程序会调用makecontext函数的第二个参数指向的函数，如果func函数返回,则恢复makecontext第一个参数指向的上下文第一个参数指向的上下文context_t中指向的uc_link.如果uc_link为NULL,则线程退出。
     void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
 makecontext修改通过getcontext取得的上下文ucp(这意味着调用makecontext前必须先调用getcontext)。
 然后给该上下文指定一个栈空间ucp->stack，设置后继的上下文ucp->uc_link.
 当上下文通过setcontext或者swapcontext激活后，执行func函数，argc为func的参数个数，后面是func的参数序列。
 当func执行返回后，继承的上下文被激活，如果继承上下文为NULL时，线程退出。
     int swapcontext(ucontext_t *oucp, ucontext_t *ucp);
 保存当前上下文到oucp结构体中，然后激活upc上下文。 
 如果执行成功，getcontext返回0，setcontext和swapcontext不返回；如果执行失败，getcontext,setcontext,swapcontext返回-1，并设置对于的errno.

 简单说来，   getcontext获取当前上下文，setcontext设置当前上下文，swapcontext切换上下文，makecontext创建一个新的上下文。


 虽然我们称协程是一个用户态的轻量级线程，但实际上多个协程同属一个线程。任意一个时刻，同一个线程不可能同时运行两个协程。


 本系统：thread ----> main_fiber(主协程)   <------> sub fiber(子协程)
            子协程之间不能之间切换，必须通过主协程 
 * @version 0.1
 * @date 2022-08-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <memory>
#include <functional>
#include <ucontext.h>


namespace leileilei
{

class Scheduler;

/**
 * @brief 协程类
 * 
 */
class Fiber : public std::enable_shared_from_this<Fiber>
{
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;
    
    /**
     * @brief 协程状态
     */
    enum State
    {
        // 初始化
        INIT,
        // 暂停
        HOLD,
        // 执行中
        EXEC,
        // 结束
        TERM,
        // 可执行
        READY,
        // 异常
        EXCEPT
    };
private:
    /**
     * @brief Construct a new Fiber object
     *     私有化的默认构造函数
        这种私有化的构造函数，可以通过类的静态函数进行调用
        通过该函数生成的fiber都是主协程
     */
    Fiber();
public:
    /**
     * @brief Construct a new Fiber object
     *  有参数的构造函数，执行该构造函数生成的fiber都是子协程，可以去执行协程绑定的函数
     * @param cb 
     * @param stacksize 
     * @param use_caller 
     */
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    /**
     * @brief Destroy the Fiber object
     * 析构函数，释放资源, 协程管理
     */
    ~Fiber();
    /**
     * @brief 重置协程函数，并设置状态
     */
    void reset(std::function<void()> cb);

    // swapIn 与 swapOut是一套
    /**
     * @brief 将 调用 的协程设置为当前正在运行的协程
     *  运行当前协程，挂起调度协程(是调度模块的调度协程)
     */
    void swapIn();
    /**
     * @brief  将主协程设置为当前正在运行的协程
        运行调度协程(是调度模块的调度协程)，挂起之前的协程
     */
    void swapOut();

    /**
     *   调度协程 和 线程的主协程，在大部分时候是相同的。但是当主线程进入线程池中则不一样。
     */

    // call 与 back是一套
    /**
     * @brief 
     * 将 调用 的协程设置为当前正在运行的协程
         运行当前协程，挂起线程主协程
     */
    void call();
    /**
     * @brief 
     * 将主协程设置为当前正在运行的协程
       运行线程主协程，挂起之前的协程
     */
    void back();

    /**
     * @brief Get the Id object
     *  返回协程id
     * @return uint64_t 
     */
    uint64_t getId();
    /**
     * @brief Get the State object
     *  返回协程的状态
     * @return State 
     */
    State getState() { return state_;}
public:
    /**
     * @brief Set the This object
     *  将参数传入的协程设置为正在运行的协程
     * @param f 
     */
    static void SetThis(Fiber* f);
    /**
     * @brief Get the This object
     *  获取当前正在执行的协程
     * @return Fiber::ptr 
     */
    static Fiber::ptr GetThis();
    /**
     * @brief 
     *  将 当前的 协程切换到后台，并且设置为ready状态  调用的是swapOut()
     */
    static void YieldToReady();
    /**
     * @brief 
     *  将 当前的 协程切换到后台，并且设置为hold状态    调用的是swapOut()
     */
    static void YieldToHold();
    /**
     * @brief 
     *  将 当前的 协程切换到后台，并且设置为ready状态  调用的是back()
     */
    static void YieldToReadyMainFiber();
    /**
     * @brief 
     *  将 当前的 协程切换到后台，并且设置为hold状态    调用的是back()
     */
    static void YieldToHoldMainFiber();

    /**
     * @brief 
     * 获取协程总数
     * @return uint64_t 
     */
    static uint64_t TotalFibers();
    /**
     * @brief 
     * 协程执行函数，执行结束应该返回到调度协程
     */
    static void MainFunc();
    /**
     * @brief 
     * 协程执行函数，执行结束应该返回到主协程
     */
    static void CallerMainFunc();
    /**
     * @brief Get the Fiber Id object
     * 获取当前的协程id
     */
    static uint64_t GetFiberId();
public:
    // 协程id
    uint64_t id_ = 0;
    // 协程运行的栈大小
    uint32_t stacksize_ = 0;
    // 协程的状态
    State state_ = INIT;
    // 协程上下文
    ucontext_t ctx_;
    // 协程运行的栈
    void* stack_ = nullptr;
    // 协程所运行的函数
    std::function<void()> callback_;

};



}

 


 