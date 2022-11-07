/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-26 16:13:03
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-07 17:14:53
 */

#include <dlfcn.h>
#include "config.h"
#include "fd_manager.h"
#include "fiber.h"
#include "hook.h"
#include "iomanager.h"
#include "log.h"


static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

namespace leileilei
{

static leileilei::ConfigVar<int>::ptr g_tcp_connect_timeout = leileilei::ConfigManager::LookUp("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;
static uint64_t s_connect_timeout = -1;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

// 用于初始化
void for_hook_init()
{
    static bool is_init = false;
    if(is_init)
    {
        return;
    }
/**
 * @brief 
 * dlsym    使用dlsym要引入 dl动态库
 */
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

// 用于初始化
struct _HookInit
{
    _HookInit()
    {
        for_hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();
        g_tcp_connect_timeout->addCallBack([](const int& old_value, const int& new_value){
            LEI_LOG_DEBUG(g_logger) << "tcp connect timeout change from " << old_value << "to " << new_value;
            s_connect_timeout = new_value;
        });
    }
};
// 用于初始化   静态全局变量的构造函数在main函数执行之前执行
static _HookInit __init__;


bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

}


// 用于连接超时
struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name, uint32_t event, int timeout_so, Args&&... args)
{
    if(!leileilei::is_hook_enable())
    {
        /**
        它会将输入的参数原封不动地传递到下一个函数中，这个“原封不动”指的是，
        如果输入的参数是左值，那么传递给下一个函数的参数的也是左值；
        如果输入的参数是右值，那么传递给下一个函数的参数的也是右值。
            template <class... Args>
            void forward(Args&&... args) {
                f(std::forward<Args>(args)...);
            } 
         */
        return fun(fd, std::forward<Args>(args)...);
    }
    LEI_LOG_DEBUG(g_logger) << "do_io ----" << hook_fun_name;
    leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(fd);
    
    if(!ctx)
        return fun(fd, std::forward<Args>(args)...);

    if(ctx->isClosed())
    {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNoblock())
        return fun(fd, std::forward<Args>(args)...);
    
    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> spti(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    /**
    如果错误为EINTR表示在读/写的时候出现了中断错误
    read()如果读到数据为0，那么就表示文件结束了，如果在读的过程中遇到了中断那么会返回-1，同时置errno为EINTR。
    或者是write()如果写的过程中遇到中断就会返回-1 并设置errno为EINTR
     */
    while(n == -1 && errno == EINTR)
        n = fun(fd, std::forward<Args>(args)...);
    /**
        当前不可读写
     */
    if(n == -1 && errno == EAGAIN)
    {
        leileilei::IOManager* iom = leileilei::IOManager::GetThis();
        leileilei::Timer::ptr timer;
        std::weak_ptr<timer_info> wpti(spti);

        if(to != (uint64_t)-1)
        {
            timer = iom->addConditionTimer(to, wpti, [wpti, fd, iom, event](){
                auto t = wpti.lock();
                if(!t || t->cancelled)
                    return;
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (leileilei::IOManager::Event)event);
            });
        }

        int rt = iom->addEvent(fd, (leileilei::IOManager::Event)event);
        // 绑定失败直接返回错误
        if(LEILEILEI_UNLIKELY(rt))
        {
            LEI_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer)
                timer->cancel();
            return -1;
        }
        else
        {
            leileilei::Fiber::YieldToHold();
            /**
             * 这里可以由定时器回调函数进入，也可以是由fd绑定的事件进入 
             */
            if(timer)
                timer->cancel();
            if(spti->cancelled)
            {
                errno = spti->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}




extern "C"
{

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds)
{
    if(!leileilei::is_hook_enable())
    {
        return sleep_f(seconds);
    }
    // 获取当前正常执行的协程
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    /**
     * 这里想绑定的其实是Scheduler::schedule函数
     但是iom是 IOManager类型
     */
    iom->addTimer(seconds * 1000, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec)
{
    if(!leileilei::is_hook_enable())
    {
        return usleep_f(usec);
    }
    // 获取当前正常执行的协程
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;   
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    if(!leileilei::is_hook_enable())
    {
        return nanosleep_f(req, rem);
    }
    // 获取当前正常执行的协程
    int timeout_ns = req->tv_sec*1000 + req->tv_nsec / 1000 / 1000;
    leileilei::Fiber::ptr fiber = leileilei::Fiber::GetThis();
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    iom->addTimer(timeout_ns, std::bind((void(leileilei::Scheduler::*)(leileilei::Fiber::ptr, int thread))&leileilei::IOManager::schedule, iom, fiber, -1));
    leileilei::Fiber::YieldToHold();
    return 0;   
}

int socket(int domain, int type, int protocol)
{
    /**
    socket()函数创建的socket默认是阻塞的
    将socket设置为非阻塞模式有两种办法:
    1、在创建socket时，指定创建的socket为非阻塞（type参数中设置SOCK_NONBLOCK标志）
    2、使用fcntl()和ioctl()函数设置socket为非阻塞模式
     */
    if(!leileilei::is_hook_enable())
        return socket_f(domain, type, protocol);
    int fd = socket_f(domain, type, protocol);
    if(fd == -1)
        return fd;
    leileilei::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms)
{
    if(!leileilei::is_hook_enable())
        return connect_f(fd, addr, addrlen);
    leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isClosed())
    {
        errno = EBADF;
        return -1;
    }
    // 不是socket还连接有意义么？
    if(!ctx->isSocket())
        return connect_f(fd, addr, addrlen);
    // 非阻塞直接返回？ 存疑
    if(ctx->getUserNoblock())
        return connect_f(fd, addr, addrlen);
        
    int n = connect_f(fd, addr, addrlen);
    if(n == 0)
        return 0;
    // EINPROGRESS  (C++11) //操作已在进行
    else if(n != -1 || errno != EINPROGRESS)
        return n;
    
    leileilei::IOManager* iom = leileilei::IOManager::GetThis();
    leileilei::Timer::ptr timer;
    std::shared_ptr<timer_info> spti(new timer_info);
    /**
    C++11标准虽然将weak_ptr定位为智能指针的一种，但该类型指针通常不单独使用（实际没有用处），只能和shared_ptr类型指针搭配使用。
    甚至于，我们可以将weak_ptr类型指针视为shared_ptr指针的一种辅助工具，借助weak_ptr类型指针，我们可以获取shared_ptr指针的一些状态信息，
    比如有多少指向相同的shared_ptr指针，shared_ptr指针指向的堆内存是否已经被释放等等。
    需要注意的是，当weak_ptr类型指针的指向和某一shared_ptr指针相同时，weak_ptr指针并不会使所指堆内存的引用计数加1；
    同样，当weak_ptr指针被释放时，之前所指堆内存的引用计数也不会因此而减1。也就是说，weak_ptr类型指针并不会影响所指堆内存空间的引用计数。
    除此之外，weak_ptr<T>模板类中没有重载*和->运算符，这也就意味着，weak_ptr类型指针只能访问所指的堆内存，而无法修改它。
     */
    std::weak_ptr<timer_info> wpti(spti);

    if(timeout_ms != (uint64_t)-1)
    {
        timer = iom->addConditionTimer(timeout_ms, wpti, [wpti, fd, iom](){
            // lock()函数，如果当前weak_ptr已经过期（指针为空，或者指向的堆内存已经被释放），则该函数会返回一个空的shared_ptr指针；
            // 反之，该函数返回一个和当前weak_ptr指针指向相同的shared_ptr指针。
            auto t = wpti.lock();
            if(!t || t->cancelled)
                return ;
            t->cancelled = ETIMEDOUT;
            // LEI_LOG_DEBUG(g_logger) << "in timer cb";
            iom->cancelEvent(fd, leileilei::IOManager::WRITE);
        });
    }

    int rt = iom->addEvent(fd, leileilei::IOManager::WRITE);
    // 添加事件成功
    if(rt == 0)
    {
        leileilei::Fiber::YieldToHold();
        if(timer)
            timer->cancel();
        if(spti->cancelled)
        {
            errno = spti->cancelled;
            return -1;
        }
    }
    else
    {
        if(timer)
            timer->cancel();
        LEI_LOG_DEBUG(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    // 连接超时
    return connect_with_timeout(sockfd, addr, addrlen, leileilei::s_connect_timeout);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int fd = do_io(sockfd, accept_f, "accept", leileilei::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0)
        leileilei::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", leileilei::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", leileilei::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", leileilei::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", leileilei::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", leileilei::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", leileilei::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", leileilei::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", leileilei::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", leileilei::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", leileilei::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd)
{
    if(!leileilei::is_hook_enable())
        return close_f(fd);
    leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(fd);
    if(ctx)
    {
        // fd关闭，关闭fd上绑定的所有事件
        auto iom = leileilei::IOManager::GetThis();
        if(iom)
            iom->cancelAll(fd);
        // 删除fd
        leileilei::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ )
{
    va_list va;
    va_start(va, cmd);
    switch(cmd)
    {
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClosed() || !ctx->isSocket())
                return fcntl_f(fd, cmd, arg);
            ctx->setUserNoblock(arg & O_NONBLOCK);
            if(ctx->getSysNoblock())
                arg |= O_NONBLOCK;
            else
                arg &= ~O_NONBLOCK;
            return fcntl_f(fd, cmd, arg);
            break;
        }
        case F_GETFL:
        {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClosed() || !ctx->isSocket())
                return arg;
            if(ctx->getUserNoblock())
                return arg | O_NONBLOCK;
            else
                return arg & ~O_NONBLOCK;
            break;
        }
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        #ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...)
{
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNoblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}


int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if(!leileilei::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    // 套接字级别上设置选项
    if(level == SOL_SOCKET) {
        /**
         * @brief Construct a new if object
         * SO_RCVTIMEO，设置接收超时时间。该选项最终将接收超时时间赋给sock->sk->sk_rcvtimeo。
            SO_SNDTIMEO，设置发送超时时间。该选项最终将发送超时时间赋给sock->sk->sk_sndtimeo。
         */
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            leileilei::FdCtx::ptr ctx = leileilei::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}
