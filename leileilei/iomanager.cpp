/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-26 10:54:23
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-30 10:07:51
 */
#include "iomanager.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>


namespace leileilei
{

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event)
{
    switch(event)
    {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            LEILEILEI_ASSERT2(false, "IOManager FdContext getContext error");
    }
    throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(EventContext& ctx)
{
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event)
{
    LEILEILEI_ASSERT(events & event);
    events = (Event)(events & ~event);
    EventContext& fd_event = getContext(event);
    if(fd_event.cb)
    {
        fd_event.scheduler->schedule(&fd_event.cb);
    }
    else
    {
        fd_event.scheduler->schedule(&fd_event.fiber);
    }
    fd_event.scheduler = nullptr;
}

IOManager::IOManager(size_t thread, bool use_caller, const std::string& name)
:Scheduler(thread, use_caller, name)
{
    // 初始化epoll
    epoll_fd_ = epoll_create(5000);
    LEILEILEI_ASSERT(epoll_fd_ > 0);

    // 初始化管道
    int rt = pipe(pipe_fd_);
    LEILEILEI_ASSERT(!rt);

    // 将管道放入到epol中
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = pipe_fd_[0];
    rt = fcntl(pipe_fd_[0], F_SETFL, O_NONBLOCK);
    LEILEILEI_ASSERT(!rt);
    
    rt = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, pipe_fd_[0], &event);
    LEILEILEI_ASSERT(!rt);

    // 初始化事件队列
    contextResize(32);
    // 启动调度器
    start();
}

IOManager::~IOManager()
{
    stop();
    close(epoll_fd_);
    close(pipe_fd_[0]);
    close(pipe_fd_[1]);

    for(size_t i=0; i<fdContexts_.size(); i++)
    {
        if(fdContexts_[i])  delete fdContexts_[i];
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
{
    // 得到fd句柄对应的事件类
    FdContext* fct = nullptr;
    RWMutexType::ReadLock lock(rwmutex_);
    if(fd < (int)fdContexts_.size())
    {
        fct = fdContexts_[fd];
        lock.unlock();
    }
    else
    {
        lock.unlock();
        RWMutexType::WriteLock lock2(rwmutex_);
        contextResize(fd * 1.5);
        fct = fdContexts_[fd];
    }

    // 确定epoll event的事件
    FdContext::MutexType::Lock lock2(fct->mutex);
    // 如果要添加的状态已经存在,则报错
    if(LEILEILEI_UNLIKELY(fct->events & event))
    {
        LEI_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                    << " event=" << (EPOLL_EVENTS)event
                    << " fct.event=" << (EPOLL_EVENTS)fct->events;
        LEILEILEI_ASSERT(!(fct->events & event));
    }

    // 放入epoll中
    int op = fct->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epollevent;
    epollevent.events = EPOLLET | fct->events | event;
    epollevent.data.ptr = fct;
    int rt = epoll_ctl(epoll_fd_, op, fd, &epollevent);
    if(rt)
    {
        // LEI_LOG_ERROR(g_logger) << "epoll_ctl(" << epoll_fd_ << ", "
        //     << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epollevent.events << "):"
        //     << rt << " (" << errno << ") (" << strerror(errno) << ") fct->events="
        //     << (EPOLL_EVENTS)fct->events;
        return -1;
    }
    event_counts_++;
    // 修改fct状态
    fct->events = (Event)(fct->events | event);
    FdContext::EventContext& fct_event = fct->getContext(event);
    // 状态检查
    LEILEILEI_ASSERT(!fct_event.scheduler && !fct_event.fiber && !fct_event.cb);
    // 绑定回调函数
    fct_event.scheduler = Scheduler::GetThis();
    if(cb)
    {
        fct_event.cb.swap(cb);
    }
    else
    {
        fct_event.fiber = Fiber::GetThis();
        LEILEILEI_ASSERT2(fct_event.fiber->getState() == Fiber::EXEC
                      ,"state=" << fct_event.fiber->getState());
    }

    return 0;
}

bool IOManager::delEvent(int fd, Event event)
{
    // 获取fd对应的事件
    RWMutexType::ReadLock lock(rwmutex_);
    if(fd >= fdContexts_.size())
    {
        LEI_LOG_ERROR(g_logger) << "fd not exit fd_list, delete event:"<< event <<" on fd="<< fd <<" error!";
        return false;
    }
    FdContext* fct = fdContexts_[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fct->mutex);
    if(LEILEILEI_UNLIKELY(!(fct->events & event)))
    {
        LEI_LOG_ERROR(g_logger) << "fd[" << fd << "] not exit event[" << event << "]";
        return false;
    }
    // 确定epoll event的事件
    Event new_events = (Event)(fct->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epollevent;
    epollevent.events = EPOLLET | new_events;
    epollevent.data.ptr = fct;

    int rt = epoll_ctl(epoll_fd_, op, fd, &epollevent);
    if(rt)
    {
        // LEI_LOG_ERROR(g_logger) << "epoll_ctl(" << epoll_fd_ << ", "
        //     << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epollevent.events << "):"
        //     << rt << " (" << errno << ") (" << strerror(errno) << ") fct->events="
        //     << (EPOLL_EVENTS)fct->events;
        return false;
    }
    event_counts_--;
    // 修改状态
    fct->events = new_events;
    FdContext::EventContext& fd_event = fct->getContext(event);
    fct->resetContext(fd_event);
    return true;
}

bool IOManager::cancelEvent(int fd, Event event)
{
    // 获取fd对应的事件
    RWMutexType::ReadLock lock(rwmutex_);
    if(fd >= fdContexts_.size())
    {
        LEI_LOG_ERROR(g_logger) << "fd not exit fd_list, delete event:"<< event <<" on fd="<< fd <<" error!";
        return false;
    }
    FdContext* fct = fdContexts_[fd];
    lock.unlock();

    // 确定epoll event事件
    FdContext::MutexType::Lock lock2(fct->mutex);
    if(LEILEILEI_UNLIKELY(!(fct->events & event)))
    {
        LEI_LOG_ERROR(g_logger) << "fd[" << fd << "] not exit event[" << event << "]";
        return false;
    }

    Event new_event = (Event)(fct->events & ~event);
    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epollevent;
    epollevent.events = EPOLLET | new_event;
    epollevent.data.ptr = fct;

    // 放入epoll
    int rt = epoll_ctl(epoll_fd_, op, fd, &epollevent);
    if(rt)
    {
        // LEI_LOG_ERROR(g_logger) << "epoll_ctl(" << epoll_fd_ << ", "
        //     << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epollevent.events << "):"
        //     << rt << " (" << errno << ") (" << strerror(errno) << ") fct->events="
        //     << (EPOLL_EVENTS)fct->events;
        return false;
    }

    fct->triggerEvent(event);
    event_counts_--;
    return true;
}   

bool IOManager::cancelAll(int fd)
{
    // 获取fd对应的事件
    RWMutexType::ReadLock lock(rwmutex_);
    if(fd >= fdContexts_.size())
    {
        LEI_LOG_ERROR(g_logger) << "fd not exit fd_list";
        return false;
    }
    FdContext* fct = fdContexts_[fd];
    lock.unlock();

    // 确定epoll event事件
    FdContext::MutexType::Lock lock2(fct->mutex);
    if(LEILEILEI_UNLIKELY(!fct->events))
    {
        LEI_LOG_ERROR(g_logger) << "fd[" << fd << "] not exit any event!";
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epollevent;
    epollevent.events = NONE;
    epollevent.data.ptr = fct;

    // 放入epoll
    int rt = epoll_ctl(epoll_fd_, op, fd, &epollevent);
    if(rt)
    {
        // LEI_LOG_ERROR(g_logger) << "epoll_ctl(" << epoll_fd_ << ", "
        //     << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epollevent.events << "):"
        //     << rt << " (" << errno << ") (" << strerror(errno) << ") fct->events="
        //     << (EPOLL_EVENTS)fct->events;
        return false;
    }

    if(fct->events & READ)
    {
        fct->triggerEvent(READ);
        event_counts_--;
    }
    if(fct->events & WRITE)
    {
        fct->triggerEvent(WRITE);
        event_counts_--;
    }
    LEILEILEI_ASSERT(fct->events == NONE);
    return true;
}

IOManager* IOManager::GetThis()
{
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::contextResize(size_t size)
{
    fdContexts_.resize(size);

    for(size_t i=0; i<size; i++)
    {
        if(!fdContexts_[i])
        {
            fdContexts_[i] = new FdContext();
            fdContexts_[i]->fd = i;
        }
    }
}

void IOManager::tickle()
{
    if(!hasIdleThreads())
    {
        LEI_LOG_DEBUG(g_logger) << "idle idle_thread_count_ is 0";
        return;
    }
    int rt = write(pipe_fd_[1], "T", 1);
    LEI_LOG_DEBUG(g_logger) << "iomanager tickle";
    LEILEILEI_ASSERT(rt == 1);
}

bool IOManager::canStop()
{
    return event_counts_==0 && Scheduler::canStop();
}

void IOManager::idle()
{
    LEI_LOG_DEBUG(g_logger) << "iomanager idle";
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS];\
    // 将events智能指针化，方便析构
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });

    while(true)
    {
        uint64_t next_timeout = 0;
        // 当可以停止时，不会进入
        if(LEILEILEI_UNLIKELY(canStop()))
        {
            LEI_LOG_DEBUG(g_logger) << "schedule name = "<< getName()<< " idle can stop, thread exit!";
            break;
        }

        int rt = 0;
        do{
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                                ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            // 阻塞等待
            rt = epoll_wait(epoll_fd_, events, MAX_EVENTS, (int)next_timeout);
            LEI_LOG_DEBUG(g_logger) << "after epoll_wait";
            // 通过返回值的状态判断是否有事件发生
            if(rt <= 0 && errno == EINTR)    {    }
            else
            {
                LEI_LOG_DEBUG(g_logger) << "rt========" << rt;
                break;
            }
        }while(true);

        for(int i=0; i<rt; i++)
        {
            epoll_event& event = events[i];
            // 读到管道fd则不做任何事情
            if(event.data.fd = pipe_fd_[0])
            {
                LEI_LOG_DEBUG(g_logger) << "come to pipe";
                uint8_t dummy[256];
                while(read(pipe_fd_[0], dummy, sizeof(dummy)) > 0);
                LEI_LOG_DEBUG(g_logger) << "out to pipe";
                continue;
            }
            // 取出fd对应的事件类
            FdContext* fct = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fct->mutex);
            
            LEI_LOG_DEBUG(g_logger) << "iomanager idel get fd=" << fct->fd;
            
            // 找到是哪种事件返回了
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fct->events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN)
            {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT)
            {
                real_events |= WRITE;
            }

            if((fct->events & real_events) == NONE)
            {
                continue;
            }

            int left_event = (Event)(fct->events & ~real_events);
            int op = left_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_event;
            int rt2 = epoll_ctl(epoll_fd_, op, fct->fd, &event);
            if(rt2)
            {
                // LEI_LOG_ERROR(g_logger) << "epoll_ctl(" << epoll_fd_ << ", "
                //     << (EpollCtlOp)op << ", " << fct->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                //     << rt << " (" << errno << ") (" << strerror(errno) << ") fct->events="
                //     << (EPOLL_EVENTS)fct->events;
                return ;
            }

            // 处理事件
            if(real_events & READ)
            {
                fct->triggerEvent(READ);
                event_counts_--;
            }
            if(real_events & WRITE)
            {
                fct->triggerEvent(WRITE);
                event_counts_--;
            }
        }
        Fiber::ptr cur_fiber = Fiber::GetThis();
        auto raw_swap = cur_fiber.get();
        cur_fiber.reset();

        raw_swap->swapOut();
    }
}

}