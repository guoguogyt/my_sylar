/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-15 10:17:08
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-19 16:59:13
 */
#include "socket.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
#include <limits.h>

namespace leileilei
{
static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

Socket::ptr Socket::CreateTCP(leileilei::Address::ptr address)
{
    Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDP(leileilei::Address::ptr address)
{
    Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
    sock->newSock();
    // 因为UDP么有连接的保证，所以只要是开启UDP默认就是已连接的
    sock->isConnect_ = true;
    return sock;
}

Socket::ptr Socket::CreateTCPIPv4Socket()
{
    Socket::ptr sock(new Socket(IPv4, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPIPv4Socket()
{
    Socket::ptr sock(new Socket(IPv4, UDP, 0));
    sock->newSock();
    sock->isConnect_ = true;
    return sock;    
}

Socket::ptr Socket::CreateTCPIPv6Socket()
{
    Socket::ptr sock(new Socket(IPv6, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPIPv6Socket()
{
    Socket::ptr sock(new Socket(IPv6, UDP, 0));
    sock->newSock();
    sock->isConnect_ = true;
    return sock;    
}

Socket::ptr Socket::CreateUnixTCPSocket()
{
    Socket::ptr sock(new Socket(UNIX, TCP, 0));
    return sock;    
}

Socket::ptr Socket::CreateUnixUDPSocket()
{
    Socket::ptr sock(new Socket(UNIX, UDP, 0));
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    :sock_(-1)
    ,family_(family)
    ,type_(type)
    ,protocol_(protocol)
    ,isConnect_(false)
{}

Socket::~Socket()
{
    close();
}

int64_t Socket::getSendTimeout()
{
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock_);
    if(ctx)
        return ctx->getTimeout(SO_SNDTIMEO);
    return -1;    
}

void Socket::setSendTimeout(uint64_t v)
{
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::getRecvTimeout()
{
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock_);
    if(ctx)
        return ctx->getTimeout(SO_RCVTIMEO);
    return -1;   
}

void Socket::setRecvTimeout(uint64_t v)
{
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOption(int level, int option, void* result, socklen_t* len)
{
    /**
     *  #include <sys/socket.h>
        int getsockopt(int socket, int level, int option_name,
            void *restrict option_value, socklen_t *restrict option_len);
        功能：获取一个套接字的选项
        参数：
            socket：文件描述符
            level：协议层次
                    SOL_SOCKET 套接字层次
                    IPPROTO_IP ip层次
                    IPPROTO_TCP TCP层次
            option_name：选项的名称（套接字层次）
                SO_BROADCAST 是否允许发送广播信息
                SO_REUSEADDR 是否允许重复使用本地地址
                SO_SNDBUF 获取发送缓冲区长度
                SO_RCVBUF 获取接收缓冲区长度    

                SO_RCVTIMEO 获取接收超时时间
                SO_SNDTIMEO 获取发送超时时间
            option_value：获取到的选项的值
            option_len：value的长度
        返回值：
            成功：0
            失败：-1
     */
    int rt = getsockopt(sock_, level, option, result, len);
    if(rt)
    {
        LEI_LOG_ERROR(g_logger) << "getOption sock=" << sock_
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int option, const void* result, socklen_t len)
{
    if(setsockopt(sock_, level, option, result, len))
    {
        LEI_LOG_ERROR(g_logger) << "setOption sock=" << sock_
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::bind(const Address::ptr addr)
{
    if(isValid())
    {
        newSock();
        if(LEILEILEI_UNLIKELY(!isValid()))
            return false;
    }

    if(LEILEILEI_UNLIKELY(addr->getFamily() != family_))
    {
        LEI_LOG_ERROR(g_logger) << "bind sock.family("
            << family_ << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
    if(uaddr)
    {
        Socket::ptr sock = Socket::CreateUnixTCPSocket();
        if(sock->connect(uaddr))
            return false;
        else
            // todo
            return true;
    }

    if(::bind(sock_, addr->getAddr(), addr->getAddrLen()))
    {
        LEI_LOG_ERROR(g_logger) << "bind error errrno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }

    getlocalAddress();
    return true;
}

/**
TCP中有如下两个队列：
SYN队列（半连接队列）：当服务器端收到客户端的SYN报文时，会响应SYN/ACK报文，然后连接就会进入SYN RECEIVED状态，
    处于SYN RECEIVED状态的连接被添加到SYN队列，并且当它们的状态改变为ESTABLISHED时，即当接收到3次握手中的ACK分组时，
    将它们移动到accept队列。SYN队列的大小由内核参数/proc/sys/net/ipv4/tcp_max_syn_backlog设置。
accept队列（完全连接队列）：accept队列存放的是已经完成TCP三次握手的连接，而accept系统调用只是简单地从accept队列中取出连接而已，
    并不是调用accept函数才会完成TCP三次握手，accept队列的大小可以通过listen函数的第二个参数控制。
 */
bool Socket::listen(int backlog)
{
    if(isValid())
    {
        LEI_LOG_ERROR(g_logger) << "listen error sock=-1";
        return false;
    }

    if(::listen(sock_, backlog))
    {
        LEI_LOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept()
{
    Socket::ptr sock(new Socket(family_, type_, protocol_));
    int newsock = ::accept(sock_, nullptr, nullptr);
    if(newsock == -1)
    {
        LEI_LOG_ERROR(g_logger) << "accept(" << sock_ << ") errno="
            << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if(sock->init(newsock))
        return sock;
    return nullptr;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms)
{
    remoteAddress_ = addr;
    if(!isValid())
    {
        newSock();
        if(LEILEILEI_UNLIKELY(!isValid()))
            return false;
    }
    if(LEILEILEI_UNLIKELY(addr->getFamily() != family_))
    {
        LEI_LOG_ERROR(g_logger) << "connect sock.family("
            << family_ << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if(timeout_ms == (uint64_t)-1)
    {
        if(::connect(sock_, addr->getAddr(), addr->getAddrLen()))
        {
            LEI_LOG_ERROR(g_logger) << "sock=" << sock_ << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    else
    {
        if(::connect_with_timeout(sock_, addr->getAddr(), addr->getAddrLen(), timeout_ms))
        {
            LEI_LOG_ERROR(g_logger) << "sock=" << sock_ << " connect(" << addr->toString()
                << ") timeout=" << timeout_ms << " error errno="
                << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }

    isConnect_ = true;
    getRemoteAddress();
    getlocalAddress();
    return true;
}

bool Socket::reconnect(uint64_t timeout_ms)
{
    if(remoteAddress_)
    {
        LEI_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
        return false;
    }
    localAddress_.reset();
    return connect(remoteAddress_, timeout_ms);
}

bool Socket::close()
{
    if(!isConnect_ && sock_ == -1)
        return true;
    isConnect_ = false;
    if(sock_ != -1)
    {
        ::close(sock_);
        sock_ = -1;
    }
    return false;
}

int Socket::send(const void* buffer, size_t length, int flags)
{
    if(isConnected())
        return ::send(sock_, buffer, length, flags);
    return -1;
}

int Socket::send(const iovec* buffer, size_t length, int flags)
{
    if(isConnected())
    {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffer;
        msg.msg_iovlen = length;
        return ::sendmsg(sock_, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to, int flags)
{
    if(isConnected())
        return ::sendto(sock_, buffer, length, flags, to->getAddr(), to->getAddrLen());
    return -1;
}

int Socket::sendTo(const iovec* buffer, size_t length, const Address::ptr to, int flags)
{
    if(isConnected())
    {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffer;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(sock_, &msg, flags);
    }    
    return -1;
}

int Socket::recv(void* buffer, size_t length, int flags)
{
    if(isConnected())
        return ::recv(sock_, buffer, length, flags);
    return -1;
}

int Socket::recv(iovec* buffer, size_t length, int flags)
{
    if(isConnected())
    {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffer;
        msg.msg_iovlen = length;
        return ::recvmsg(sock_, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags)
{
    if(isConnected())
    {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(sock_, buffer, length, flags, from->getAddr(), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec* buffer, size_t length, Address::ptr from, int flags)
{
    if(isConnected())
    {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffer;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(sock_, &msg, flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress()
{
    if(remoteAddress_)
        return remoteAddress_;
    
    Address::ptr result;
    switch(family_)
    {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(family_));
    }
    socklen_t addrlen = result->getAddrLen();
    /**
    #include<sys/socket.h>
     
    getsockname函数用于获取与某个套接字关联的本地协议地址
    int getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen);
    
    getpeername只有在连接建立以后才调用，否则不能正确获得对方地址和端口，所以它的参数描述字一般是已连接描述字而非监听套接口描述字。
    没有连接的UDP不能调用getpeername，但是可以调用getsockname和TCP一样，它的地址和端口不是在调用socket就指定了，而是在第一次调用sendto函数以后。
    已经连接的UDP，在调用connect以后，这2个函数（getsockname，getpeername）都是可以用的。
    但是这时意义不大，因为已经连接（connect）的UDP已经知道对方的地址。
    getpeername函数用于获取与某个套接字关联的外地协议地址
    int getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen);
    调用成功，则返回0，如果调用出错，则返回-1

    在一个没有调用bind的TCP客户上，connect成功返回后，getsockname用于返回由内核赋予该连接的本地IP地址和本地端口号。
　  在以端口号为0调用bind（告知内核去选择本地临时端口号）后，getsockname用于返回由内核赋予的本地端口号。
　  在一个以通配IP地址调用bind的TCP服务器上，与某个客户的连接一旦建立（accept成功返回），getsockname就可以用于返回由内核赋予该连接的本地IP地址。
    在这样的调用中，套接字描述符参数必须是已连接套接字的描述符，而不是监听套接字的描述符。
　  当一个服务器的是由调用过accept的某个进程通过调用exec执行程序时，它能够获取客户身份的唯一途径便是调用getpeername。
     */
    if(getpeername(sock_, result->getAddr(), &addrlen))
        return Address::ptr(new UnknownAddress(family_));
    
    if(family_ == AF_UNIX)
    {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    remoteAddress_ = result;
    return remoteAddress_;
}

Address::ptr Socket::getlocalAddress()
{
    if(localAddress_)
        return localAddress_;
    Address::ptr result;
    switch(family_)
    {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(family_));
            break;
    }

    socklen_t addrlen = result->getAddrLen();
    if(getsockname(sock_, result->getAddr(), &addrlen))
    {
        LEI_LOG_ERROR(g_logger) << "getsockname error sock=" << sock_
            << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknownAddress(family_));
    }
    if(family_ == AF_UNIX)
    {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    localAddress_ = result;
    return localAddress_;
}

bool Socket::isValid() const
{
    return sock_ != -1;
}

int Socket::getError()
{
    int error = 0;
    socklen_t len = sizeof(error);
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len))
        error = errno;
    return error;
}

std::ostream& Socket::dump(std::ostream& os) const
{
    os << "[Socket sock=" << sock_
       << " is_connected=" << isConnect_
       << " family=" << family_
       << " type=" << type_
       << " protocol=" << protocol_;
    if(localAddress_) {
        os << " local_address=" << localAddress_->toString();
    }
    if(remoteAddress_) {
        os << " remote_address=" << remoteAddress_->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const
{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

bool Socket::cancelRead()
{
    return IOManager::GetThis()->cancelEvent(sock_, leileilei::IOManager::READ);
}

bool Socket::cancelWrite()
{
    return IOManager::GetThis()->cancelEvent(sock_, leileilei::IOManager::WRITE);
}

bool Socket::cancelAccept()
{
    return IOManager::GetThis()->cancelEvent(sock_, leileilei::IOManager::READ);
}

bool Socket::cancelAll()
{
    return IOManager::GetThis()->cancelAll(sock_);
}

void Socket::initSock()
{
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if(type_ == SOCK_STREAM)
        /**
         * @brief 连接建立后通常会设置一个选项：TCP_NODELAY，该选项会禁用Nagle算法。
            Nagle算法的作用是减少小包的数量
                什么是小包：小于 MSS(一个TCP段在网络上传输的最大尺寸) 的都可以定义为小包。
                如果前一个TCP段发送出去后，还没有收到对端的ACK包，那么接下来的发送数据会先累积起来不发。
                等到对端返回ACK，或者数据累积已快达到MSS，才会发送出去。
            目的降低时延
         */
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
}

void Socket::newSock()
{
    sock_ = socket(family_, type_, protocol_);
    if(LEILEILEI_LIKELY(sock_ != 1))
        initSock();
    else
        LEI_LOG_ERROR(g_logger) << "socket(" << family_
            << ", " << type_ << ", " << protocol_ << ") errno="
            << errno << " errstr=" << strerror(errno);
}

bool Socket::init(int sock)
{
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock_);
    if(ctx && ctx->isSocket() && !ctx->isClosed())
    {
        sock_ = sock;
        isConnect_ = true;
        initSock();
        getlocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

}