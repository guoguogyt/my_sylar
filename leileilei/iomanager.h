/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-26 10:54:13
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-12 09:34:28
 */
#pragma

#include "scheduler.h"
#include "timer.h"

namespace leileilei
{

/**
 * @brief 基于epoll的IO事件协程调度器
 * 本质上还是一个协程调度器
 */
class IOManager : public Scheduler, public TimerManager
{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    
    /**
     * @brief IO事件
     */
    enum Event
    {
        //  无事件
        NONE = 0x0,
        // 读事件
        READ = 0x1,
        // 写事件
        WRITE = 0x4,
    };
private:
    // socket事件上下文
    struct FdContext
    {
        typedef Mutex MutexType;
        // 事件上下文
        struct EventContext
        {
            Scheduler* scheduler = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;
        };
        
        /**
         * @brief Get the Context object
         * 获取事件的上下文
         * @param event 事件类型
         * @return EventContext& 
         */
        EventContext& getContext(Event event);

        /**
         * @brief 重置事件的上下文
         * 
         * @param ctx 
         */
        void resetContext(EventContext& ctx);

        /**
         * @brief 触发事件
         * 
         * @param event 事件类型
         */
        void triggerEvent(Event event);

        // 读事件的上下文
        EventContext read;
        // 写事件的上下文
        EventContext write;
        // 事件关联句柄
        int fd = 0;
        // 当前的事件
        Event events = NONE;
        MutexType mutex;
    };
public:
    /**
     * @brief Construct a new IOManager object
     * 参数与Scheduler的构造函数相同，要做一些准备工作,启动调度器
     * @param thread 
     * @param use_caller 
     * @param name 
     */
    IOManager(size_t thread = 1, bool use_caller = true, const std::string& name = "");
    /**
     * @brief Destroy the IOManager object
     * 关闭调度器
     */
    ~IOManager();
    /**
     * @brief 添加一个事件
     * @param fd socket句柄
     * @param event 事件类型
     * @param cb 事件的回调函数
     * @return int 
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    /**
     * @brief 删除事件 
     * @param fd socket句柄
     * @param event 事件类型
     * @return 成功返回0，失败返回-1 
     */
    bool delEvent(int fd, Event event);
    /**
     * @brief 取消事件，如果事件存在则要触发事件
     * @param fd socket句柄
     * @param event 事件类型
     * @return true 
     */
    bool cancelEvent(int fd, Event event);
    /**
     * @brief 取消所有的事件
     * @param fd 
     * @return true 
     * @return false 
     */
    bool cancelAll(int fd);
    /**
     * @brief Get the This object
     * 返回当前的IOManager
     * @return IOManager* 
     */
    static IOManager* GetThis();
protected:
    void tickle()   override;
    bool canStop()  override;
    bool canStop(uint64_t& timeout);
    void idle() override;
    void onFrontTimer() override;
    /**
     * @brief 
     * 重置socket句柄上下文的容器大小
     * @param size 
     */
    void contextResize(size_t size);
private:
    // epoll的句柄
    int epoll_fd_ = 0;
    // 管道，用于唤醒epoll
    int pipe_fd_[2];
    // 等待执行的事件数量
    std::atomic<size_t> event_counts_ = {0};
    RWMutexType rwmutex_;
    // socket的上下文容器
    std::vector<FdContext*> fdContexts_;
};



}

