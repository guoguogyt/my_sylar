/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-26 16:12:58
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-03 14:02:08
 */

#pragma once
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace leileilei
{
    /**
     * @brief 
     *  是否开启hook
     * @return true 
     * @return false 
     */
    bool is_hook_enable();
    /**
     * @brief Set the hook enable object
     *  
     * @return true 
     * @return false 
     */
    void set_hook_enable(bool flag);
}

/**
 * @brief 
 *  使用C的编译风格
    因为C和C++的编译规则不一样，主要区别体现在编译期间生成函数符号的规则不一致。
    C++比C出道晚，但是增加了很多优秀的功能，函数重载就是其中之一。
    由于C++需要支持重载，单纯的函数名无法区分出具体的函数，所以在编译阶段就需要将形参列表作为附加项增加到函数符号中。
 */
extern "C"
{
// sleep  参数是秒
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;
// usleep 参数是微秒
typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;
// nanosleep 纳秒级别
typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
extern nanosleep_fun nanosleep_f;


// socket       error -1
typedef int (*socket_fun)(int domain, int type, int protocol);
extern socket_fun socket_f;
// connnect     success 0           error -1
typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern connect_fun connect_f;
// accept    success nonnegative integer            error -1
typedef int (*accept_fun)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern accept_fun accept_f;


// read         error -1
typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;
typedef ssize_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
extern readv_fun readv_f;

typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
extern recv_fun recv_f;

typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_fun recvfrom_f;

typedef ssize_t (*recvmsg_fun)(int sockfd, struct msghdr *msg, int flags);
extern recvmsg_fun recvmsg_f;

//write
typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
extern write_fun write_f;

typedef ssize_t (*writev_fun)(int fd, const struct iovec *iov, int iovcnt);
extern writev_fun writev_f;

typedef ssize_t (*send_fun)(int s, const void *msg, size_t len, int flags);
extern send_fun send_f;

typedef ssize_t (*sendto_fun)(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
extern sendto_fun sendto_f;

typedef ssize_t (*sendmsg_fun)(int s, const struct msghdr *msg, int flags);
extern sendmsg_fun sendmsg_f;

typedef int (*close_fun)(int fd);
extern close_fun close_f;

/**
 * @brief 
 * fcntl函数有5种功能： 
1. 复制一个现有的描述符(cmd=F_DUPFD). 
2. 获得／设置文件描述符标记(cmd=F_GETFD或F_SETFD). 
3. 获得／设置文件状态标记(cmd=F_GETFL或F_SETFL). 
4. 获得／设置异步I/O所有权(cmd=F_GETOWN或F_SETOWN). 
5. 获得／设置记录锁(cmd=F_GETLK , F_SETLK或F_SETLKW).
 */
typedef int (*fcntl_fun)(int fd, int cmd, ... /* arg */ );
extern fcntl_fun fcntl_f;

/**
 * @brief 
 * ioctl是设备驱动程序中对设备的I/O通道进行管理的函数。所谓对I/O通道进行管理，就是对设备的一些特性进行控制，例如串口的传输波特率、马达的转速等等。
　　ioctl函数是文件结构中的一个属性分量，就是说如果你的驱动程序提供了对ioctl的支持，用户就可以在用户程序中使用ioctl函数来控制设备的I/O通道。
　　用户程序所作的只是通过命令码(cmd)告诉驱动程序它想做什么，至于怎么解释这些命令和怎么实现这些命令，这都是驱动程序要做的事情。
 */
typedef int (*ioctl_fun)(int d, unsigned long int request, ...);
extern ioctl_fun ioctl_f;


typedef int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
extern getsockopt_fun getsockopt_f;

/**
 * @brief 
 * 可以设置socket读写超时
 */
typedef int (*setsockopt_fun)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
extern setsockopt_fun setsockopt_f;

extern int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms);


}
